/*!
 * \file      main.c
 *
 * \brief     Performs a periodic uplink
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */

/*! \file periodic-uplink/B-L072Z-LRWAN1/main.c */

#include <stdio.h>
#include "utilities.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"

#include "cli.h"
#include "Commissioning.h"
#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "CayenneLpp.h"
#include "LmHandlerMsgDisplay.h"

#include "delay.h"
#include "config.h"
#include "ublox.h"
#include "bsp.h"
#include "geofence.h"
#include "playback.h"
#include "nvmm.h"
#include "iwdg.h"
#include "print_utils.h"
#include "LoRaWAN_config_switcher.h"

#include "callbacks.h"

#ifndef ACTIVE_REGION

#warning "No active region defined, LORAMAC_REGION_EU868 will be used as default."

#define ACTIVE_REGION LORAMAC_REGION_EU868

#endif

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
uint32_t APP_TX_DUTYCYCLE;

/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND 1000

/*!
 * LoRaWAN Adaptive Data Rate
 *
 * \remark Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE LORAMAC_HANDLER_ADR_OFF

/*!
 * Default datarate
 *
 * \remark Please note that LORAWAN_DEFAULT_DATARATE is used only when ADR is disabled 
 */
#define LORAWAN_DEFAULT_DATARATE DR_4

/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON true

/*!
 * LoRaWAN application port
 * @remark The allowed port range is from 1 up to 223. Other values are reserved.
 */
#define LORAWAN_APP_PORT 99

/*!
 *
 */
typedef enum
{
    LORAMAC_HANDLER_TX_ON_TIMER,
    LORAMAC_HANDLER_TX_ON_EVENT,
} LmHandlerTxEvents_t;

/*!
 * User application data
 */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/*!
 * User application data structure
 */
static LmHandlerAppData_t AppData =
    {
        .Buffer = AppDataBuffer,
        .BufferSize = 0,
        .Port = 0};

/*!
 * Specifies the state of the application LED
 */
static bool AppLedStateOn = false;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

/**
 * @brief Timer to handle watchdog kicking
 * 
 */
static TimerEvent_t WatchdogKickTimer;

static void OnMacProcessNotify(void);
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

static void PrepareTxFrame(void);
static void StartTxProcess(LmHandlerTxEvents_t txEvent);
static void UplinkProcess(void);

static void print_board_info();
static void init_loramac(picotracker_lorawan_settings_t settings);

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void *context);

static void OnWatchdogKickTimerEvent(void *context);

static void retrieve_lorawan_region(void);

typedef enum
{
    geofence_reinit_pending,
    no_issue_carry_on,
} loop_status_t;

int setup_board(void);
loop_status_t run_loop_once(void);
int init_loramac_stack_and_tx_scheduling(void);
void loop(void);
void update_geofence_status();
void do_transmission();
void WatchdogProcess(void);

static LmHandlerCallbacks_t LmHandlerCallbacks =
    {
        .GetBatteryLevel = BoardGetBatteryLevel,
        .GetTemperature = NULL,
        .GetRandomSeed = BoardGetRandomSeed,
        .OnMacProcess = OnMacProcessNotify,
        .OnNvmDataChange = OnNvmDataChange,
        .OnNetworkParametersChange = OnNetworkParametersChange,
        .OnMacMcpsRequest = OnMacMcpsRequest,
        .OnMacMlmeRequest = OnMacMlmeRequest,
        .OnJoinRequest = OnJoinRequest,
        .OnTxData = OnTxData,
        .OnRxData = OnRxData,
        .OnClassChange = OnClassChange,
        .OnBeaconStatusChange = OnBeaconStatusChange,
        .OnSysTimeUpdate = OnSysTimeUpdate,
};

static LmHandlerParams_t LmHandlerParams =
    {
        .Region = ACTIVE_REGION,
        .AdrEnable = LORAWAN_ADR_STATE,
        .TxDatarate = LORAWAN_DEFAULT_DATARATE,
        .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK,
        .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
        .DataBufferMaxSize = LORAWAN_APP_DATA_BUFFER_MAX_SIZE,
        .DataBuffer = AppDataBuffer,
        .is_over_the_air_activation = false,
};

static LmhpComplianceParams_t LmhpComplianceParams =
    {
        .AdrEnabled = LORAWAN_ADR_STATE,
        .DutyCycleEnabled = LORAWAN_DUTYCYCLE_ON,
        .StopPeripherals = NULL,
        .StartPeripherals = NULL,
};

/*!
 * Indicates if LoRaMacProcess call is pending.
 * 
 * \warning If variable is equal to 0 then the MCU can be set in low power mode
 */
static volatile uint8_t IsMacProcessPending = 0;

static volatile uint8_t IsTxFramePending = 0;

static volatile bool IsWatchdogKickPending = false;
/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1; // Tx

/*!
 * UART object used for command line interface handling
 */
extern Uart_t Uart1;

#ifdef UNITTESTING_LORA
extern TimerTime_t current_time;
#endif

uint32_t tx_count = 0;
uint32_t n_tx_per_network = 3;

/*!
 * Main application entry point.
 */
#ifndef UNITTESTING_LORA
int main(void)
#else
int run_app(void)
#endif
{

    setup_board();

#if PRINT_EEPROM_DEBUG
    printf("Dumping EEPROM:\n");
    EEPROM_Dump();
    printf("\n");
#endif

    loop();
}

void loop()
{
    while (1)
    {

        /* reading sensors and GPS */
        BSP_sensor_Read();

        gps_info_t latest = get_latest_gps_info();
        if (latest.latest_gps_status == GPS_SUCCESS)
        {
            update_geofence_status();
        }

        if (current_geofence_status.tx_permission == TX_OK)
        {
            do_transmission();
        }
    }
}

void do_transmission()
{
    init_loramac_stack_and_tx_scheduling();

    TimerInit(&WatchdogKickTimer, OnWatchdogKickTimerEvent);
    OnWatchdogKickTimerEvent(NULL);

    tx_count = 0;
    while (1)
    {
        run_loop_once();

        if (tx_count > n_tx_per_network)
        {
            break;
        }
    }
}

void update_geofence_status()
{
    /* Find out which region of world we are in and update region parm*/

    gps_info_t gps_info = get_latest_gps_info();
    update_geofence_position(gps_info.GPS_UBX_latitude_Float, gps_info.GPS_UBX_longitude_Float);

    /* Save current polygon to eeprom only if gps fix was valid */
    NvmmWrite((void *)&current_geofence_status.current_loramac_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);

    IWDG_reset();
}

int init_loramac_stack_and_tx_scheduling()
{
    const char *region_string = get_lorawan_region_string(current_geofence_status.current_loramac_region);
    printf("Initialising Loramac Stack with Loramac region: %s\n", region_string);

    picotracker_lorawan_settings_t settings = get_otaa_abp_setting(current_geofence_status.current_loramac_region);
    APP_TX_DUTYCYCLE = settings.tx_interval;

    init_loramac(settings);

    //LmHandlerJoin( );

    /* Start up periodic timer for sending uplinks */
    StartTxProcess(LORAMAC_HANDLER_TX_ON_TIMER);

    return EXIT_SUCCESS;
}

loop_status_t run_loop_once()
{
#ifdef UNITTESTING_LORA
    current_time += 1; /* simulate 1 millisecond per loop */

    TimerIrqHandler();
#endif

    WatchdogProcess();

    // Process characters sent over the command line interface
    CliProcess(&Uart1);

    // Processes the LoRaMac events
    LmHandlerProcess();

    // Process application uplinks management
    UplinkProcess();

    IWDG_reset();

    CRITICAL_SECTION_BEGIN();
    if (IsMacProcessPending == 1)
    {
        // Clear flag and prevent MCU to go into low power modes.
        IsMacProcessPending = 0;
    }
    else
    {
        // The MCU wakes up through events
        BoardLowPowerHandler();
    }
    CRITICAL_SECTION_END();

    return no_issue_carry_on;
}

int setup_board()
{
    /* Get reset cause for diagnosis */
    reset_cause_t reset_cause = reset_cause_get();

    /* Initialising board and peripherals */
    BoardInitMcu();
    BoardInitPeriph();

    /* Print reset cause after print function initialised */
    printf("\n\nThe system reset cause is \"%s\"\n", reset_cause_get_name(reset_cause));

    /* Print board info */
    print_board_info();

    /* Get initial GPS fix for setting loramac region */
    retrieve_lorawan_region();

    return EXIT_SUCCESS;
}

void WatchdogProcess(void)
{
    IWDG_reset();
    IsWatchdogKickPending = false;
}

static void print_board_info()
{

    const Version_t appVersion = {.Fields.Major = 1, .Fields.Minor = 0, .Fields.Patch = 0};
    const Version_t gitHubVersion = {.Fields.Major = 4, .Fields.Minor = 4, .Fields.Patch = 7};
    DisplayAppInfo("periodic-uplink-lpp",
                   &appVersion,
                   &gitHubVersion);
}

static void init_loramac(picotracker_lorawan_settings_t settings)
{

    /* Set region and datarate */
    LmHandlerParams.Region = settings.region;
    LmHandlerParams.TxDatarate = settings.datarate;
    LmHandlerParams.is_over_the_air_activation = settings.is_over_the_air_activation;

    if (LmHandlerInit(&LmHandlerCallbacks, &LmHandlerParams) != LORAMAC_HANDLER_SUCCESS)
    {
        printf("LoRaMac wasn't properly initialized\n");
        // Fatal error, endless loop.
        while (1)
        {
        }
    }

    // Set system maximum tolerated rx error in milliseconds
    LmHandlerSetSystemMaxRxError(20);

    // The LoRa-Alliance Compliance protocol package should always be
    // initialized and activated.
    LmHandlerPackageRegister(PACKAGE_ID_COMPLIANCE, &LmhpComplianceParams);
}

static void retrieve_lorawan_region()
{

    IWDG_reset();

    /* read the eeprom value instead */
    // TODO: must ensure that eeprom is not filled with garbage. i.e. when the eeprom has never been programed
    if (USE_NVM_STORED_LORAWAN_REGION == true)
    {
        NvmmRead((void *)&current_geofence_status.current_loramac_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);
    }
    IWDG_reset();
}

static void OnMacProcessNotify(void)
{
    IsMacProcessPending = 1;
}

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
    DisplayRxUpdate(appData, params);

    switch (appData->Port)
    {
    case 1: // The application LED can be controlled on port 1 or 2
    case LORAWAN_APP_PORT:
    {
        AppLedStateOn = appData->Buffer[0] & 0x01;
    }
    break;

    case DOWNLINK_CONFIG_PORT:
    {

        printf("Received data: ");
        print_bytes(appData->Buffer, appData->BufferSize);
        manage_incoming_instruction(appData->Buffer);
    }
    break;

    default:
        break;
    }
}

/*!
 * Prepares the payload of the frame and transmits it.
 */
static void PrepareTxFrame(void)
{
    if (LmHandlerIsBusy() == true)
    {
        return;
    }

    tx_count++;

    if (tx_count > n_tx_per_network)
    {
        return;
    }

    PicoTrackerAppData_t data = prepare_tx_buffer();

    AppData.Port = LORAWAN_APP_PORT;
    AppData.Buffer = data.Buffer;
    AppData.BufferSize = data.BufferSize;

    // Print out buffer for debug
    printf("Buffer to tx:\n");
    print_bytes(AppData.Buffer, AppData.BufferSize);
    printf("tx_str_buffer_len: %d\n\n", AppData.BufferSize);

    const char *region_string = get_lorawan_region_string(current_geofence_status.current_loramac_region);
    printf("Loramac region: %s \n", region_string);

    if (LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE) == LORAMAC_HANDLER_SUCCESS)
    {
        // Switch LED 1 ON
    }
}

static void StartTxProcess(LmHandlerTxEvents_t txEvent)
{
    switch (txEvent)
    {
    default:
        // Intentional fall through
    case LORAMAC_HANDLER_TX_ON_TIMER:
    {
        // Schedule 1st packet transmission
        TimerInit(&TxTimer, OnTxTimerEvent);

        TimerStop(&TxTimer);
        IsTxFramePending = 1;
        // Schedule next transmission
        TimerSetValue(&TxTimer, 5); /* Start first tranmission 5 milliseconds after initalisation */
        TimerStart(&TxTimer);
    }
    break;
    case LORAMAC_HANDLER_TX_ON_EVENT:
    {
    }
    break;
    }
}

static void UplinkProcess(void)
{
    uint8_t isPending = 0;
    CRITICAL_SECTION_BEGIN();
    isPending = IsTxFramePending;
    IsTxFramePending = 0;
    CRITICAL_SECTION_END();
    if (isPending == 1)
    {
        PrepareTxFrame();
    }
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void *context)
{
    TimerStop(&TxTimer);

    IsTxFramePending = 1;

    // Schedule next transmission
    TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND));
    TimerStart(&TxTimer);
}

static void OnWatchdogKickTimerEvent(void *context)
{
    TimerStop(&WatchdogKickTimer);
    IsWatchdogKickPending = true;
    TimerSetValue(&WatchdogKickTimer, WATCH_DOG_KICK_INTERVAL);
    TimerStart(&WatchdogKickTimer);
}

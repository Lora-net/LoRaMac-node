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
#include "board.h"

#include "cli.h"
#include "Commissioning.h"

#include "config.h"
#include "ublox.h"
#include "geofence.h"
#include "iwdg.h"
#include "print_utils.h"
#include "NvmDataMgmt.h"

#include "callbacks.h"
#include "message_sender.h"

#include "nvmm.h"

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
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxTimer;

typedef enum
{
    geofence_reinit_pending,
    no_issue_carry_on,
} loop_status_t;

static void OnMacProcessNotify(void);
static void StartTxProcess(LmHandlerTxEvents_t txEvent);
static void UplinkProcess(void);
static void init_loramac(picotracker_lorawan_settings_t settings);

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void *context);

loop_status_t run_loop_once(void);
void run_main_loop(void);
void do_n_transmissions(uint32_t n_transmissions_todo);

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

/*!
 * UART object used for command line interface handling
 */
extern Uart_t Uart1;

#ifdef UNITTESTING_LORA
extern TimerTime_t current_time;
#endif

uint32_t tx_count = 0;
uint32_t n_tx_per_network = 1;

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

    run_main_loop();
}

void run_main_loop()
{

    while (1)
    {
        do_n_transmissions(2); // do 2 transmissions on the same network
    }
}

void do_n_transmissions(uint32_t n_transmissions_todo)
{
    n_tx_per_network = n_transmissions_todo;

    switch_to_next_registered_credentials(); // Credentials have been set. So now tell it that next time credentials are called for, use the next set.

    print_current_region();
    picotracker_lorawan_settings_t settings = get_lorawan_setting(get_current_loramac_region());

    init_loramac(settings);

    LmHandlerJoin();

    /* Start up periodic timer for sending uplinks */
    StartTxProcess(LORAMAC_HANDLER_TX_ON_TIMER);

    IWDG_reset();

    tx_count = 0;
    while (1)
    {
        run_loop_once();

        if (tx_count > n_transmissions_todo)
        {
            break;
        }
    }
}

loop_status_t run_loop_once()
{
#ifdef UNITTESTING_LORA
    current_time += 1; /* simulate 1 millisecond per loop */

    TimerIrqHandler();
#endif

    // Process characters sent over the command line interface
    CliProcess(&Uart1);

    // Processes the LoRaMac events
    LmHandlerProcess();

    // Process application uplinks management
    UplinkProcess();

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

static void init_loramac(picotracker_lorawan_settings_t settings)
{

    /* Set region and datarate */
    LmHandlerParams.Region = get_current_loramac_region();
    LmHandlerParams.TxDatarate = settings.datarate;

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

static void OnMacProcessNotify(void)
{
    IsMacProcessPending = 1;
}

void setup_next_tx_alarm()
{

    TimerStop(&TxTimer); /* Stop tx timer. Requirement before starting it back up again */
    TimerSetValue(&TxTimer, read_tx_interval_in_eeprom());
    TimerStart(&TxTimer); /* Restart tx interval timer */
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

    if (tx_count < n_tx_per_network)
    {
        sensor_read_and_send(&AppData, LmHandlerParams.Region);
    }

    setup_next_tx_alarm();
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
        IWDG_reset();
        PrepareTxFrame();
        IWDG_reset();
    }
}

/*!
 * Function executed on TxTimer event
 */
static void OnTxTimerEvent(void *context)
{
    TimerStop(&TxTimer);

    IsTxFramePending = 1;
}

/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
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
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "stm32l0xx.h"
#include "utilities.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "sysIrqHandlers.h"
#include "board-config.h"
#include "lpm-board.h"
#include "rtc-board.h"
#include "sx1276-board.h"
#include "board.h"

#include "bsp.h"
#include "config.h"
#include "delay.h"
#include "iwdg.h"

/*!
 * Unique Devices IDs register set ( STM32L0xxx )
 */
#define ID1 (0x1FF80050)
#define ID2 (0x1FF80054)
#define ID3 (0x1FF80064)

/**
 * ADC defines
 * 
 */
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_5
#define VDDA_VREFINT_CAL ((uint32_t)3000)
#define VREFINT_CAL ((uint16_t *)((uint32_t)0x1FF80078))

/* Internal temperature sensor: constants data used for indicative values in  */
/* this example. Refer to device datasheet for min/typ/max values.            */

/* Internal temperature sensor, parameter TS_CAL1: TS ADC raw data acquired at 
 *a temperature of 110 DegC (+-5 DegC), VDDA = 3.3 V (+-10 mV). */
#define TEMP30_CAL_ADDR ((uint16_t *)((uint32_t)0x1FF8007A))

/* Internal temperature sensor, parameter TS_CAL2: TS ADC raw data acquired at 
 *a temperature of  30 DegC (+-5 DegC), VDDA = 3.3 V (+-10 mV). */
#define TEMP110_CAL_ADDR ((uint16_t *)((uint32_t)0x1FF8007E))

/* Vdda value with which temperature sensor has been calibrated in production 
   (+-10 mV). */
#define VDDA_TEMP_CAL ((uint32_t)3000)

#define COMPUTE_TEMPERATURE(TS_ADC_DATA, VDDA_APPLI) \
    (((((((int32_t)((TS_ADC_DATA * VDDA_APPLI) / VDDA_TEMP_CAL) - (int32_t)*TEMP30_CAL_ADDR)) * (int32_t)(110 - 30)) << 8) / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR)) + (30 << 8))

/*!
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;
Gpio_t Led4;

Gpio_t Gps_int;
Gpio_t Load_enable;

Gpio_t i2c_scl;
Gpio_t i2c_sda;

/*
 * MCU objects
 */
Uart_t Uart1;
I2c_t I2c;
Adc_t Adc;

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit(void);

/*!
 * System Clock Configuration
 */
static void SystemClockConfig(void);

/*!
 * Used to measure and calibrate the system wake-up time from STOP mode
 */
static void CalibrateSystemWakeupTime(void);

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig(void);

/*!
 * Timer used at first boot to calibrate the SystemWakeupTime
 */
static TimerEvent_t CalibrateSystemWakeupTimeTimer;

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/*!
 * Flag used to indicate if board is powered from the USB
 */
static bool UsbIsConnected = false;

/*!
 * UART1 FIFO buffers size
 */
#define UART1_FIFO_TX_SIZE 1024
#define UART1_FIFO_RX_SIZE 1024

uint8_t Uart1TxBuffer[UART1_FIFO_TX_SIZE];
uint8_t Uart1RxBuffer[UART1_FIFO_RX_SIZE];

/*!
 * Flag to indicate if the SystemWakeupTime is Calibrated
 */
static volatile bool SystemWakeupTimeCalibrated = false;

/*!
 * Callback indicating the end of the system wake-up time calibration
 */
static void OnCalibrateSystemWakeupTimeTimerEvent(void *context)
{
    SystemWakeupTimeCalibrated = true;
}

void BoardCriticalSectionBegin(uint32_t *mask)
{
    *mask = __get_PRIMASK();
    __disable_irq();
}

void BoardCriticalSectionEnd(uint32_t *mask)
{
    __set_PRIMASK(*mask);
}

void BoardInitPeriph(void)
{
    BSP_sensor_Init();
}

void BoardInitMcu(void)
{
}

void BoardResetMcu(void)
{
    CRITICAL_SECTION_BEGIN();

    //Restart system
    NVIC_SystemReset();
}

void BoardDeInitMcu(void)
{
}

uint32_t BoardGetRandomSeed(void)
{
    return ((*(uint32_t *)ID1) ^ (*(uint32_t *)ID2) ^ (*(uint32_t *)ID3));
}

void BoardGetUniqueId(uint8_t *id)
{
    id[7] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 24;
    id[6] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 16;
    id[5] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 8;
    id[4] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3));
    id[3] = ((*(uint32_t *)ID2)) >> 24;
    id[2] = ((*(uint32_t *)ID2)) >> 16;
    id[1] = ((*(uint32_t *)ID2)) >> 8;
    id[0] = ((*(uint32_t *)ID2));
}

uint16_t BoardBatteryMeasureVoltage(void)
{
    return 0;
}

uint32_t BoardGetBatteryVoltage(void)
{
    uint16_t batteryLevel = 3232;

    return batteryLevel;
}

uint8_t BoardGetBatteryLevel(void)
{
    return 0;
}

static void BoardUnusedIoInit(void)
{
}

void SystemClockConfig(void)
{
}

void CalibrateSystemWakeupTime(void)
{
    if (SystemWakeupTimeCalibrated == false)
    {
        TimerInit(&CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent);
        TimerSetValue(&CalibrateSystemWakeupTimeTimer, 1000);
        TimerStart(&CalibrateSystemWakeupTimeTimer);
        while (SystemWakeupTimeCalibrated == false)
        {
        }
    }
}

void SystemClockReConfig(void)
{
}

void SysTick_Handler(void)
{
}

uint8_t GetBoardPowerSource(void)
{
    if (UsbIsConnected == false)
    {
        return BATTERY_POWER;
    }
    else
    {
        return USB_POWER;
    }
}

/**
  * \brief Enters Low Power Stop Mode
  *
  * \note ARM exists the function when waking up
  */
void LpmEnterStopMode(void)
{
}

/*!
 * \brief Exists Low Power Stop Mode
 */
void LpmExitStopMode(void)
{
    // Disable IRQ while the MCU is not running on HSI
    CRITICAL_SECTION_BEGIN();

    // Initilizes the peripherals
    BoardInitMcu();

    CRITICAL_SECTION_END();
}

/*!
 * \brief Enters Low Power Sleep Mode
 *
 * \note ARM exits the function when waking up
 */
void LpmEnterSleepMode(void)
{
}

void BoardLowPowerHandler(void)
{
}

#if !defined(__CC_ARM)

/*
 * Function to be used by stdout for printf etc
 */
int _write(int fd, const void *buf, size_t count)
{

    return count;
}

/*
 * Function to be used by stdin for scanf etc
 */
int _read(int fd, const void *buf, size_t count)
{
    size_t bytesRead = 0;

    return bytesRead;
}

#else

#include <stdio.h>

// Keil compiler
int fputc(int c, FILE *stream)
{
    while (UartPutChar(&Uart1, (uint8_t)c) != 0)
        ;
    return c;
}

int fgetc(FILE *stream)
{
    uint8_t c = 0;
    while (UartGetChar(&Uart1, &c) != 0)
        ;
    // Echo back the character
    while (UartPutChar(&Uart1, c) != 0)
        ;
    return (int)c;
}

#endif

#ifdef USE_FULL_ASSERT

#include <stdio.h>

/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %lu\n", file, line) */

    printf("Wrong parameters value: file %s on line %lu\n", (const char *)file, line);
    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
 * @brief 
 * @param[in] reset_cause The previously-obtained system reset cause
 * @return A null-terminated ASCII name string describing the system reset cause
 */
const char *reset_cause_get_name(reset_cause_t reset_cause)
{
    const char *reset_cause_name = "TBD";

    switch (reset_cause)
    {
    case RESET_CAUSE_UNKNOWN:
        reset_cause_name = "UNKNOWN";
        break;
    case RESET_CAUSE_LOW_POWER_RESET:
        reset_cause_name = "LOW_POWER_RESET";
        break;
    case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
        reset_cause_name = "WINDOW_WATCHDOG_RESET";
        break;
    case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
        reset_cause_name = "INDEPENDENT_WATCHDOG_RESET";
        break;
    case RESET_CAUSE_SOFTWARE_RESET:
        reset_cause_name = "SOFTWARE_RESET";
        break;
    case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
        reset_cause_name = "POWER-ON_RESET (POR) / POWER-DOWN_RESET (PDR)";
        break;
    case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
        reset_cause_name = "EXTERNAL_RESET_PIN_RESET";
        break;
    case RESET_CAUSE_OPTIONS_BYTES_LOADING_RESET:
        reset_cause_name = "RESET_CAUSE_OPTIONS_BYTES_LOADING_RESET";
        break;
    case RESET_CAUSE_FIREWALL_RESET:
        reset_cause_name = "RESET_CAUSE_FIREWALL_RESET";
        break;
    case RESET_CAUSE_BROWNOUT_RESET:
        reset_cause_name = "BROWNOUT_RESET (BOR)";
        break;
    }

    return reset_cause_name;
}

/**
 * @brief Obtain the STM32 system reset cause
 * @param None
 * @return reset_cause_t The system reset cause
 */
reset_cause_t reset_cause_get(void)
{

    reset_cause_t reset_cause = RESET_CAUSE_UNKNOWN;

     return reset_cause;
}

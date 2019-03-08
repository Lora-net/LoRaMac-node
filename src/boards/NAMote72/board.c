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
#include "stm32l1xx.h"
#include "utilities.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "gps.h"
#include "mpl3115.h"
#include "mag3110.h"
#include "mma8451.h"
#include "sx9500.h"
#include "board-config.h"
#include "lpm-board.h"
#include "rtc-board.h"
#include "sx1272-board.h"
#include "board.h"

/*!
 * Unique Devices IDs register set ( STM32L152x )
 */
#define         ID1                                 ( 0x1FF800D0 )
#define         ID2                                 ( 0x1FF800D4 )
#define         ID3                                 ( 0x1FF800E4 )

/*!
 * LED GPIO pins objects
 */
Gpio_t LedRed;    // Active Low
Gpio_t LedYellow; // Active Low
Gpio_t LedGreen;  // Active Low
Gpio_t LedUsr;    // Active High

/*!
 * PushButton GPIO pin object
 */
Gpio_t PushButton;

/*
 * MCU objects
 */
Adc_t Adc;
I2c_t I2c;
Uart_t Uart1;
Uart_t Uart2;

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * System Clock Configuration
 */
static void SystemClockConfig( void );

/*!
 * Used to measure and calibrate the system wake-up time from STOP mode
 */
static void CalibrateSystemWakeupTime( void );

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );

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
 * UART2 FIFO buffers size
 */
#define UART2_FIFO_TX_SIZE                                2048
#define UART2_FIFO_RX_SIZE                                2048

uint8_t Uart2TxBuffer[UART2_FIFO_TX_SIZE];
uint8_t Uart2RxBuffer[UART2_FIFO_RX_SIZE];

/*!
 * Flag to indicate if the SystemWakeupTime is Calibrated
 */
static volatile bool SystemWakeupTimeCalibrated = false;

/*!
 * Callback indicating the end of the system wake-up time calibration
 */
static void OnCalibrateSystemWakeupTimeTimerEvent( void* context )
{
    RtcSetMcuWakeUpTime( );
    SystemWakeupTimeCalibrated = true;
}

/*!
 * Holds the bord version.
 */
static Version_t BoardVersion = { 0 };

void BoardCriticalSectionBegin( uint32_t *mask )
{
    *mask = __get_PRIMASK( );
    __disable_irq( );
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    __set_PRIMASK( *mask );
}

void BoardInitPeriph( void )
{
    Gpio_t ioPin;

    // Init the GPIO pins
    GpioInit( &LedRed, LED_RED, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &LedYellow, LED_YELLOW, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &LedGreen, LED_GREEN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &LedUsr, LED_USR, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    GpioInit( &PushButton, PC_0, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

    GpioInit( &ioPin, IRQ_1_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, IRQ_2_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, IRQ_MPL3115, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    // Init temperature, pressure and altitude sensor
    MPL3115Init( );

    // Init accelerometer
    MMA8451Init( );

    // Init SAR
    SX9500Init( );

    // Init GPS
    GpsInit( );

    // Switch LED 1, 2, 3, 4 OFF
    GpioWrite( &LedRed, 1 );
    GpioWrite( &LedYellow, 1 );
    GpioWrite( &LedGreen, 1 );
    GpioWrite( &LedUsr, 0 );
}

void BoardInitMcu( void )
{
    Gpio_t ioPin;

    if( McuInitialized == false )
    {
        HAL_Init( );

        SystemClockConfig( );

        GpioInit( &ioPin, UART_RX, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        if( GpioRead( &ioPin ) == 1 )   // Debug Mode
        {
            UsbIsConnected = true;
            FifoInit( &Uart2.FifoTx, Uart2TxBuffer, UART2_FIFO_TX_SIZE );
            FifoInit( &Uart2.FifoRx, Uart2RxBuffer, UART2_FIFO_RX_SIZE );
            // Configure your terminal for 8 Bits data (7 data bit + 1 parity bit), no parity and no flow ctrl
            UartInit( &Uart2, UART_2, UART_TX, UART_RX );
            UartConfig( &Uart2, RX_TX, 921600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
        }
        else
        {
            UsbIsConnected = false;
            UartDeInit( &Uart2 );
        }

        RtcInit( );

        BoardUnusedIoInit( );

        I2cInit( &I2c, I2C_1, I2C_SCL, I2C_SDA );
        if( GetBoardPowerSource( ) == BATTERY_POWER )
        {
            // Disables OFF mode - Enables lowest power mode (STOP)
            LpmSetOffMode( LPM_APPLI_ID, LPM_DISABLE );
        }
    }
    else
    {
        SystemClockReConfig( );
    }

    BoardVersion = BoardGetVersion( );
    switch( BoardVersion.Fields.Major )
    {
    case 2:
        AdcInit( &Adc, BAT_LEVEL_PIN_PA0 );
        break;
    case 3:
        AdcInit( &Adc, BAT_LEVEL_PIN_PA1 );
        break;
    default:
        break;
    }

    SpiInit( &SX1272.Spi, SPI_2, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1272IoInit( );

    if( McuInitialized == false )
    {
        McuInitialized = true;
        if( GetBoardPowerSource( ) == BATTERY_POWER )
        {
            CalibrateSystemWakeupTime( );
        }
    }
}

void BoardResetMcu( void )
{
    CRITICAL_SECTION_BEGIN( );

    //Restart system
    NVIC_SystemReset( );

}

void BoardDeInitMcu( void )
{
    Gpio_t ioPin;

    AdcDeInit( &Adc );

    SpiDeInit( &SX1272.Spi );
    SX1272IoDeInit( );

    GpioInit( &ioPin, OSC_HSE_IN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, OSC_HSE_OUT, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    GpioInit( &ioPin, OSC_LSE_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 1 );
    GpioInit( &ioPin, OSC_LSE_OUT, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 1 );
}

uint32_t BoardGetRandomSeed( void )
{
    return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
    id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[0] = ( ( *( uint32_t* )ID2 ) );
}

/*!
 * Factory power supply
 */
#define FACTORY_POWER_SUPPLY                        3300 // mV

/*!
 * VREF calibration value
 */
#define VREFINT_CAL                                 ( *( uint16_t* )0x1FF80078 )

/*!
 * ADC maximum value
 */
#define ADC_MAX_VALUE                               4095

/*!
 * Battery thresholds
 */
#define BATTERY_MAX_LEVEL                           3700 // mV
#define BATTERY_MIN_LEVEL                           1900 // mV
#define BATTERY_SHUTDOWN_LEVEL                      1800 // mV

static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

uint16_t BoardBatteryMeasureVolage( void )
{
    uint16_t vdd = 0;
    uint16_t vref = VREFINT_CAL;
    uint16_t vdiv = 0;
    uint16_t batteryVoltage = 0;

    switch( BoardVersion.Fields.Major )
    {
        case 2:
            vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL_PA0 );
            break;
        case 3:
            vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL_PA1 );
            break;
        default:
            break;
    }
    //vref = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT );

    vdd = ( float )FACTORY_POWER_SUPPLY * ( float )VREFINT_CAL / ( float )vref;
    batteryVoltage = vdd * ( ( float )vdiv / ( float )ADC_MAX_VALUE );

    //                                vDiv
    // Divider bridge  VBAT <-> 10k -<--|-->- 10k <-> GND => vBat = 2 * vDiv
    batteryVoltage = 2 * batteryVoltage;
    return batteryVoltage;
}

uint32_t BoardGetBatteryVoltage( void )
{
    return BatteryVoltage;
}

uint8_t BoardGetBatteryLevel( void )
{
    uint8_t batteryLevel = 0;

    BatteryVoltage = BoardBatteryMeasureVolage( );

    if( GetBoardPowerSource( ) == USB_POWER )
    {
        batteryLevel = 0;
    }
    else
    {
        if( BatteryVoltage >= BATTERY_MAX_LEVEL )
        {
            batteryLevel = 254;
        }
        else if( ( BatteryVoltage > BATTERY_MIN_LEVEL ) && ( BatteryVoltage < BATTERY_MAX_LEVEL ) )
        {
            batteryLevel = ( ( 253 * ( BatteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
        }
        else if( ( BatteryVoltage > BATTERY_SHUTDOWN_LEVEL ) && ( BatteryVoltage <= BATTERY_MIN_LEVEL ) )
        {
            batteryLevel = 1;
        }
        else //if( BatteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
        {
            batteryLevel = 255;
        }
    }
    return batteryLevel;
}

static void BoardUnusedIoInit( void )
{
    Gpio_t ioPin;

    if( GetBoardPowerSource( ) == USB_POWER )
    {
        HAL_DBGMCU_EnableDBGStopMode( );
        HAL_DBGMCU_EnableDBGSleepMode( );
        HAL_DBGMCU_EnableDBGStandbyMode( );
    }
    else
    {
        GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

        HAL_DBGMCU_DisableDBGSleepMode( );
        HAL_DBGMCU_DisableDBGStopMode( );
        HAL_DBGMCU_DisableDBGStandbyMode( );

        GpioInit( &ioPin, JTAG_TMS, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, JTAG_TCK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, JTAG_TDI, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, JTAG_TDO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, JTAG_NRST, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }

    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    switch( BoardVersion.Fields.Major )
    {
        case 2:
            GpioInit( &ioPin, BAT_LEVEL_PIN_PA0, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
            break;
        case 3:
            GpioInit( &ioPin, BAT_LEVEL_PIN_PA1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
            break;
        default:
            break;
    }
}

Version_t BoardGetVersion( void )
{
    Gpio_t pinPc1;
    Gpio_t pinPc7;
    Version_t boardVersion = { 0 };
    boardVersion.Value = 0;

    GpioInit( &pinPc1, BOARD_VERSION_PC1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &pinPc7, BOARD_VERSION_PC7, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    uint8_t first = GpioRead( &pinPc1 );
    GpioWrite( &pinPc7, 0 );

    if( first && !GpioRead( &pinPc1 ) )
    {
        boardVersion.Fields.Major = 2;
    }
    else
    {
        boardVersion.Fields.Major = 3;
    }
    return boardVersion;
}

void SystemClockConfig( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    __HAL_RCC_PWR_CLK_ENABLE( );

    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
    if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
    {
        assert_param( FAIL );
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 ) != HAL_OK )
    {
        assert_param( FAIL );
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
    {
        assert_param( FAIL );
    }

    HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq( ) / 1000 );

    HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

    // SysTick_IRQn interrupt configuration
    HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}

void CalibrateSystemWakeupTime( void )
{
    if( SystemWakeupTimeCalibrated == false )
    {
        TimerInit( &CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent );
        TimerSetValue( &CalibrateSystemWakeupTimeTimer, 1000 );
        TimerStart( &CalibrateSystemWakeupTimeTimer );
        while( SystemWakeupTimeCalibrated == false )
        {

        }
    }
}

void SystemClockReConfig( void )
{
    __HAL_RCC_PWR_CLK_ENABLE( );
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    // Enable HSE
    __HAL_RCC_HSE_CONFIG( RCC_HSE_ON );

    // Wait till HSE is ready
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    // Enable PLL
    __HAL_RCC_PLL_ENABLE( );

    // Wait till PLL is ready
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
    {
    }

    // Select PLL as system clock source
    __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );

    // Wait till PLL is used as system clock source
    while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
    {
    }
}

void SysTick_Handler( void )
{
    HAL_IncTick( );
    HAL_SYSTICK_IRQHandler( );
}

uint8_t GetBoardPowerSource( void )
{
    if( UsbIsConnected == false )
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
void LpmEnterStopMode( void)
{
    CRITICAL_SECTION_BEGIN( );

    BoardDeInitMcu( );

    // Disable the Power Voltage Detector
    HAL_PWR_DisablePVD( );

    // Clear wake up flag
    SET_BIT( PWR->CR, PWR_CR_CWUF );

    // Enable Ultra low power mode
    HAL_PWREx_EnableUltraLowPower( );

    // Enable the fast wake up from Ultra low power mode
    HAL_PWREx_EnableFastWakeUp( );

    CRITICAL_SECTION_END( );

    // Enter Stop Mode
    HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}

/*!
 * \brief Exists Low Power Stop Mode
 */
void LpmExitStopMode( void )
{
    // Disable IRQ while the MCU is not running on HSI
    CRITICAL_SECTION_BEGIN( );

    // Initializes the peripherals
    BoardInitMcu( );

    CRITICAL_SECTION_END( );
}

/*!
 * \brief Enters Low Power Sleep Mode
 *
 * \note ARM exits the function when waking up
 */
void LpmEnterSleepMode( void)
{
#ifndef DEBUG
    /*!
     * Temporarily remove the following call when compiling in debug mode.
     * 
     * Due to an yet unknown reason the PWR_MAINREGULATOR_ON constant gets
     * changed inside the function which makes the assert fail.
     * 
     * When compiling in release mode the code operates as expected.
     * 
     * TODO: Check what causes this issue. First guess is that the stack gets
     *       corrupted somehow.
     * 
     * This function is only called when using the GPS peripheral.
     */
    HAL_PWR_EnterSLEEPMode( PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI );
#endif
}

void BoardLowPowerHandler( void )
{
    __disable_irq( );
    /*!
     * If an interrupt has occurred after __disable_irq( ), it is kept pending 
     * and cortex will not enter low power anyway
     */

    LpmEnterLowPower( );

    __enable_irq( );
}

#if !defined ( __CC_ARM )

/*
 * Function to be used by stdout for printf etc
 */
int _write( int fd, const void *buf, size_t count )
{
    while( UartPutBuffer( &Uart2, ( uint8_t* )buf, ( uint16_t )count ) != 0 ){ };
    return count;
}

/*
 * Function to be used by stdin for scanf etc
 */
int _read( int fd, const void *buf, size_t count )
{
    size_t bytesRead = 0;
    while( UartGetBuffer( &Uart2, ( uint8_t* )buf, count, ( uint16_t* )&bytesRead ) != 0 ){ };
    // Echo back the character
    while( UartPutBuffer( &Uart2, ( uint8_t* )buf, ( uint16_t )bytesRead ) != 0 ){ };
    return bytesRead;
}

#else

// Keil compiler
int fputc( int c, FILE *stream )
{
    while( UartPutChar( &Uart2, ( uint8_t )c ) != 0 );
    return c;
}

int fgetc( FILE *stream )
{
    uint8_t c = 0;
    while( UartGetChar( &Uart2, &c ) != 0 );
    // Echo back the character
    while( UartPutChar( &Uart2, c ) != 0 );
    return ( int )c;
}

#endif

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %lu\r\n", file, line) */

    printf( "Wrong parameters value: file %s on line %lu\r\n", ( const char* )file, line );
    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif

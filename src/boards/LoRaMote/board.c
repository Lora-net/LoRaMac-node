/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

/*!
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

#if defined( USE_DEBUG_PINS )
Gpio_t DbgPin1;
Gpio_t DbgPin2;
Gpio_t DbgPin3;
Gpio_t DbgPin4;
Gpio_t DbgPin5;
#endif

/*
 * MCU objects
 */
Adc_t Adc;
I2c_t I2c;
Uart_t Uart1;
#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

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
 * Flag to indicate if the SystemWakeupTime is Calibrated
 */
static bool SystemWakeupTimeCalibrated = false;

/*!
 * Callback indicating the end of the system wake-up time calibration
 */
static void OnCalibrateSystemWakeupTimeTimerEvent( void )
{
    SystemWakeupTimeCalibrated = true;
}

/*!
 * Nested interrupt counter.
 *
 * \remark Interrupt should only be fully disabled once the value is 0
 */
static uint8_t IrqNestLevel = 0;

void BoardDisableIrq( void )
{
    __disable_irq( );
    IrqNestLevel++;
}

void BoardEnableIrq( void )
{
    IrqNestLevel--;
    if( IrqNestLevel == 0 )
    {
        __enable_irq( );
    }
}

void BoardInitPeriph( void )
{
    Gpio_t ioPin;

    // Init the GPIO pins
    GpioInit( &ioPin, IRQ_MPL3115, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, IRQ_MAG3110, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, SPARE_IO_EXT_3, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, SPARE_IO_EXT_4, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, SPARE_IO_EXT_5, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, SPARE_IO_EXT_6, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, SPARE_IO_EXT_7, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, N_IRQ_SX9500, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, IRQ_1_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, IRQ_2_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &ioPin, TX_EN_SX9500, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    // Init temperature, pressure and altitude sensor
    MPL3115Init( );

    // Init accelerometer
    MMA8451Init( );

    // Init magnetometer
    MAG3110Init( );

    // Init SAR
    SX9500Init( );

    // Init GPS
    GpsInit( );

    // Switch LED 1, 2, 3 OFF
    GpioWrite( &Led1, 1 );
    GpioWrite( &Led2, 1 );
    GpioWrite( &Led3, 1 );
}

void BoardInitMcu( void )
{
    if( McuInitialized == false )
    {
#if defined( USE_BOOTLOADER )
        // Set the Vector Table base location at 0x3000
        SCB->VTOR = FLASH_BASE | 0x3000;
#endif
        HAL_Init( );

        SystemClockConfig( );

#if defined( USE_USB_CDC )
        UartInit( &UartUsb, UART_USB_CDC, NC, NC );
        UartConfig( &UartUsb, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );

        DelayMs( 1000 ); // 1000 ms for Usb initialization
#endif

        RtcInit( );

        BoardUnusedIoInit( );

        I2cInit( &I2c, I2C_SCL, I2C_SDA );

#if defined( USE_DEBUG_PINS )
        GpioInit( &DbgPin1, CON_EXT_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin2, CON_EXT_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin3, CON_EXT_7, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin4, CON_EXT_8, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin5, CON_EXT_9, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
    }
    else
    {
        SystemClockReConfig( );
    }

    AdcInit( &Adc, BAT_LEVEL_PIN );

    SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
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
#define BATTERY_MAX_LEVEL                           9000 // mV
#define BATTERY_MIN_LEVEL                           4700 // mV
#define BATTERY_SHUTDOWN_LEVEL                      4800 // mV

static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

uint16_t BoardBatteryMeasureVolage( void )
{
    uint16_t vdd = 0;
    uint16_t vref = VREFINT_CAL;
    uint16_t vdiv = 0;
    uint16_t batteryVoltage = 0;

    vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL );
    //vref = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT );

    vdd = ( float )FACTORY_POWER_SUPPLY * ( float )VREFINT_CAL / ( float )vref;
    batteryVoltage = vdd * ( ( float )vdiv / ( float )ADC_MAX_VALUE );

    //                                vDiv
    // Divider bridge  VBAT <-> 20k -<--|-->- 10k <-> GND => vBat = 3 * vDiv
    batteryVoltage = 3 * batteryVoltage;
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

    /* External Connector J5 */
#if !defined( USE_DEBUG_PINS )
    GpioInit( &ioPin, CON_EXT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, CON_EXT_3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, CON_EXT_7, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, CON_EXT_8, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, CON_EXT_9, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    if( GetBoardPowerSource( ) == BATTERY_POWER )
    {
        GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }

    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    GpioInit( &ioPin, BAT_LEVEL_PIN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    GpioInit( &ioPin, PIN_PB6, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, WKUP1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

#if defined( USE_DEBUGGER )
    HAL_DBGMCU_EnableDBGStopMode( );
    HAL_DBGMCU_EnableDBGSleepMode( );
    HAL_DBGMCU_EnableDBGStandbyMode( );
#else
    HAL_DBGMCU_DisableDBGSleepMode( );
    HAL_DBGMCU_DisableDBGStopMode( );
    HAL_DBGMCU_DisableDBGStandbyMode( );

    GpioInit( &ioPin, JTAG_TMS, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TCK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TDI, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TDO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_NRST, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
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
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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

    // HAL_NVIC_GetPriorityGrouping
    HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );

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
            TimerLowPowerHandler( );
        }
    }
}

void SystemClockReConfig( void )
{
    __HAL_RCC_PWR_CLK_ENABLE( );
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    /* Enable HSE */
    __HAL_RCC_HSE_CONFIG( RCC_HSE_ON );

    /* Wait till HSE is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    /* Enable PLL */
    __HAL_RCC_PLL_ENABLE( );

    /* Wait till PLL is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
    {
    }

    /* Select PLL as system clock source */
    __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );

    /* Wait till PLL is used as system clock source */
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
#if defined( USE_USB_CDC )
    if( UartUsbIsUsbCableConnected( ) == 0 )
    {
        return BATTERY_POWER;
    }
    else
    {
        return USB_POWER;
    }
#else
    return BATTERY_POWER;
#endif
}

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif

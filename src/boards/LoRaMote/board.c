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
 * Battery level ratio (battery dependent)
 */
#define BATTERY_STEP_LEVEL                          0.23

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

/*!
 * IO Extander pins objects
 */
Gpio_t IrqMpl3115;
Gpio_t IrqMag3110;
Gpio_t GpsPowerEn;
Gpio_t NcIoe3;
Gpio_t NcIoe4;
Gpio_t NcIoe5;
Gpio_t NcIoe6;
Gpio_t NcIoe7;
Gpio_t NIrqSx9500;
Gpio_t Irq1Mma8451;
Gpio_t Irq2Mma8451;
Gpio_t TxEnSx9500;
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

#if defined( USE_DEBUG_PINS )
Gpio_t DbgPin1;
Gpio_t DbgPin2;
Gpio_t DbgPin3;
Gpio_t DbgPin4;
#endif

/*
 * MCU objects
 */
Gpio_t GpsPps;
Gpio_t GpsRx;
Gpio_t GpsTx;

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
 * Timer calibration
 */
static void CalibrateTimer( void );

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );

/*!
 * Timer used at first boot to calibrate the Timer
 */
static TimerEvent_t TimerCalibrationTimer;

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/*!
 * Flag to indicate if the Timer is Calibrated
 */
static bool TimerCalibrated = false;


static void OnTimerCalibrationTimerEvent( void )
{
    TimerCalibrated = true;
}

void BoardInitPeriph( void )
{
    /* Init the GPIO extender pins */
    GpioInit( &IrqMpl3115, IRQ_MPL3115, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &IrqMag3110, IRQ_MAG3110, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &NcIoe3, SPARE_IO_EXT_3, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &NcIoe4, SPARE_IO_EXT_4, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &NcIoe5, SPARE_IO_EXT_5, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &NcIoe6, SPARE_IO_EXT_6, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &NcIoe7, SPARE_IO_EXT_7, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &NIrqSx9500, N_IRQ_SX9500, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Irq1Mma8451, IRQ_1_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Irq2Mma8451, IRQ_2_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &TxEnSx9500, TX_EN_SX9500, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
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
    }
    else
    {
        SystemClockReConfig( );
    }

    AdcInit( &Adc, BAT_LEVEL );

    SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1272IoInit( );

    if( McuInitialized == false )
    {
        McuInitialized = true;
        CalibrateTimer( );
    }
}

void BoardDeInitMcu( void )
{
    Gpio_t ioPin;

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

uint8_t BoardGetBatteryLevel( void )
{
    uint8_t batteryLevel = 0;
    uint16_t measuredLevel = 0;

    measuredLevel = AdcReadChannel( &Adc );

    if( measuredLevel >= 3900 )  // 9V
    {
        batteryLevel = 254;
    }
    else
    {
        batteryLevel = ( measuredLevel - 1870 ) * BATTERY_STEP_LEVEL; // 1870 => 4.7V = limit of operation for the battery
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
    GpioInit( &ioPin, CON_EXT_9, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    if( GetBoardPowerSource( ) == BATTERY_POWER )
    {
        GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }

    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    GpioInit( &ioPin, CON_EXT_8, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, BAT_LEVEL, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

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

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 );

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );

    HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq( ) / 1000 );

    HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

    /*    HAL_NVIC_GetPriorityGrouping*/
    HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}

void CalibrateTimer( void )
{
    if( TimerCalibrated == false )
    {
        TimerInit( &TimerCalibrationTimer, OnTimerCalibrationTimerEvent );
        TimerSetValue( &TimerCalibrationTimer, 1000 );
        TimerStart( &TimerCalibrationTimer );
        while( TimerCalibrated == false )
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
    while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
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

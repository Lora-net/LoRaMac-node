/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Andreas Pella (IMST GmbH), Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * Potentiometer max and min levels definition
 */
#define POTI_MAX_LEVEL 900
#define POTI_MIN_LEVEL 10

/*!
 * Vref values definition
 */
#define PDDADC_VREF_BANDGAP                             1224 // mV
#define PDDADC_MAX_VALUE                                4096

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

#if ( USE_POTENTIOMETER == 0 )
Gpio_t Led1;
#endif
Gpio_t Led2;
Gpio_t Led3;
Gpio_t Led4;

/*
 * MCU objects
 */
Adc_t Adc;
I2c_t I2c;
Uart_t Uart1;


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
#if ( USE_POTENTIOMETER == 0 )
    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
#endif
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led4, LED_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );


    // Switch LED 1, 2, 3, 4 OFF
#if ( USE_POTENTIOMETER == 0 )
    GpioWrite( &Led1, 0 );
#endif
    GpioWrite( &Led2, 0 );
    GpioWrite( &Led3, 0 );
    GpioWrite( &Led4, 0 );
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

        RtcInit( );

        BoardUnusedIoInit( );
    }
    else
    {
        SystemClockReConfig( );
    }

    AdcInit( &Adc, POTI );

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

uint8_t BoardMeasurePotiLevel( void )
{
    uint8_t potiLevel = 0;
    uint16_t MeasuredLevel = 0;

    // read the current potentiometer setting
    MeasuredLevel = AdcMcuRead( &Adc , ADC_CHANNEL_3 );

    // check the limits
    if( MeasuredLevel >= POTI_MAX_LEVEL )
    {
        potiLevel = 100;
    }
    else if( MeasuredLevel <= POTI_MIN_LEVEL )
    {
        potiLevel = 0;
    }
    else
    {
        // if the value is in the area, calculate the percentage value
        potiLevel = ( ( MeasuredLevel - POTI_MIN_LEVEL ) * 100 ) / POTI_MAX_LEVEL;
    }
    return potiLevel;
}

uint16_t BoardMeasureVdd( void )
{
    uint16_t MeasuredLevel = 0;
    uint32_t milliVolt = 0;

    // Read the current Voltage
    MeasuredLevel = AdcMcuRead( &Adc , ADC_CHANNEL_17 );

    // We don't use the VREF from calibValues here.
    // calculate the Voltage in miliVolt
    milliVolt = ( uint32_t )PDDADC_VREF_BANDGAP * ( uint32_t )PDDADC_MAX_VALUE;
    milliVolt = milliVolt / ( uint32_t ) MeasuredLevel;

    return ( uint16_t ) milliVolt;
}

uint8_t BoardGetBatteryLevel( void )
{
    uint8_t batteryLevel = 0;
    uint16_t measuredLevel = 0;

    measuredLevel = BoardMeasureVdd( );

    if( measuredLevel >= 3000 )
    {
        batteryLevel = 254;
    }
    else if( measuredLevel <= 2400 )
    {
        batteryLevel = 1;
    }
    else
    {
        batteryLevel = ( measuredLevel - 2400 ) * BATTERY_STEP_LEVEL;
    }
    return batteryLevel;
}

static void BoardUnusedIoInit( void )
{
    Gpio_t ioPin;
    if( GetBoardPowerSource( ) == BATTERY_POWER )
    {
        GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }

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
    return BATTERY_POWER;
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

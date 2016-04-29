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

#include "adc-board.h"

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
Gpio_t RadioPushButton;
Gpio_t BoardPowerDown;
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
Gpio_t Led4;


/*
 * MCU objects
 */
Gpio_t GpsPps;
Gpio_t GpsRx;
Gpio_t GpsTx;
Gpio_t UsbDetect;
Gpio_t Wkup1;
Gpio_t DcDcEnable;
Gpio_t BatVal;

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
    GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    /* Init the GPIO extender pins */
    GpioInit( &IrqMpl3115, IRQ_MPL3115, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &IrqMag3110, IRQ_MAG3110, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &GpsPowerEn, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &RadioPushButton, RADIO_PUSH_BUTTON, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &BoardPowerDown, BOARD_POWER_DOWN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
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
    GpioInit( &Led4, LED_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

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

    // Switch LED 1, 2, 3, 4 OFF
    GpioWrite( &Led1, 1 );
    GpioWrite( &Led2, 1 );
    GpioWrite( &Led3, 1 );
    GpioWrite( &Led4, 1 );
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

        GpioInit( &UsbDetect, USB_ON, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &BatVal, BAT_LEVEL, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
    else
    {
        SystemClockReConfig( );
    }

    SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1276IoInit( );

    if( McuInitialized == false )
    {
        McuInitialized = true;
        CalibrateTimer( );
    }
}

void BoardDeInitMcu( void )
{
    Gpio_t ioPin;

    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );

    GpioInit( &ioPin, OSC_HSE_IN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, OSC_HSE_OUT, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    GpioInit( &ioPin, OSC_LSE_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, OSC_LSE_OUT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    GpioInit( &UsbDetect, USB_ON, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
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
#define FACTORY_POWER_SUPPLY                        3.0L

/*!
 * VREF calibration value
 */
#define VREFINT_CAL                                 ( *( uint16_t* )0x1FF80078 )

/*!
 * ADC maximum value
 */
#define ADC_MAX_VALUE                               4096

/*!
 * Battery thresholds
 */
#define BATTERY_MAX_LEVEL                           4150 // mV
#define BATTERY_MIN_LEVEL                           3200 // mV
#define BATTERY_SHUTDOWN_LEVEL                      3100 // mV

uint16_t BoardGetPowerSupply( void )
{
    float vref = 0;
    float vdiv = 0;
    float batteryVoltage = 0;

    AdcInit( &Adc, BAT_LEVEL );

    vref = AdcMcuRead( &Adc, ADC_CHANNEL_17 );
    vdiv = AdcMcuRead( &Adc, ADC_CHANNEL_8 );

    batteryVoltage = ( FACTORY_POWER_SUPPLY * VREFINT_CAL * vdiv ) / ( vref * ADC_MAX_VALUE );

    //                                vDiv
    // Divider bridge  VBAT <-> 1M -<--|-->- 1M <-> GND => vBat = 2 * vDiv
    batteryVoltage = 2 * batteryVoltage;

    return ( uint16_t )( batteryVoltage * 1000 );
}

uint8_t BoardGetBatteryLevel( void )
{
    volatile uint8_t batteryLevel = 0;
    uint16_t batteryVoltage = 0;

    if( GpioRead( &UsbDetect ) == 1 )
    {
        batteryLevel = 0;
    }
    else
    {
        batteryVoltage = BoardGetPowerSupply( );

        if( batteryVoltage >= BATTERY_MAX_LEVEL )
        {
            batteryLevel = 254;
        }
        else if( ( batteryVoltage > BATTERY_MIN_LEVEL ) && ( batteryVoltage < BATTERY_MAX_LEVEL ) )
        {
            batteryLevel = ( ( 253 * ( batteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
        }
        else if( batteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
        {
            batteryLevel = 255;
            //GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
            //GpioInit( &BoardPowerDown, BOARD_POWER_DOWN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
        }
        else // BATTERY_MIN_LEVEL
        {
            batteryLevel = 1;
        }
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

    GpioInit( &ioPin, TEST_POINT1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, TEST_POINT2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, TEST_POINT3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, TEST_POINT4, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    GpioInit( &ioPin, PIN_NC, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

    GpioInit( &ioPin, RF_RXTX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, WKUP1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

#if defined( USE_DEBUGGER )
    HAL_DBGMCU_EnableDBGStopMode( );
    HAL_DBGMCU_EnableDBGSleepMode( );
    HAL_DBGMCU_EnableDBGStandbyMode( );
#else
    HAL_DBGMCU_DisableDBGSleepMode( );
    HAL_DBGMCU_DisableDBGStopMode( );
    HAL_DBGMCU_DisableDBGStandbyMode( );

    GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
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
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
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
    if( GpioRead( &UsbDetect ) == 1 )
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

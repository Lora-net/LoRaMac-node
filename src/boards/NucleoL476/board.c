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
#include "stm32l4xx.h"
#include "utilities.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "board-config.h"
#include "lpm-board.h"
#include "rtc-board.h"

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    #include "sx126x-board.h"
#elif defined( SX1272MB2DAS)
    #include "sx1272-board.h"
#elif defined( SX1276MB1LAS ) || defined( SX1276MB1MAS ) || defined( SX1276UNDEFINED )
    #include "sx1276-board.h"
#endif
#include "board.h"

/*!
 * Unique Devices IDs register set ( STM32L4xxx )
 */
#define         ID1                                 ( 0x1FFF7590 )
#define         ID2                                 ( 0x1FFF7594 )
#define         ID3                                 ( 0x1FFF7594 )

/*!
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;

/*
 * MCU objects
 */
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
#define UART2_FIFO_TX_SIZE                                1024
#define UART2_FIFO_RX_SIZE                                1024

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

}

void BoardInitMcu( void )
{
    if( McuInitialized == false )
    {
        HAL_Init( );

        // LEDs
        GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

        SystemClockConfig( );

        UsbIsConnected = true;

        FifoInit( &Uart2.FifoTx, Uart2TxBuffer, UART2_FIFO_TX_SIZE );
        FifoInit( &Uart2.FifoRx, Uart2RxBuffer, UART2_FIFO_RX_SIZE );
        // Configure your terminal for 8 Bits data (7 data bit + 1 parity bit), no parity and no flow ctrl
        UartInit( &Uart2, UART_2, UART_TX, UART_RX );
        UartConfig( &Uart2, RX_TX, 921600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );

        RtcInit( );

        BoardUnusedIoInit( );
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

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    SpiInit( &SX126x.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX126xIoInit( );
#elif defined( SX1272MB2DAS)
    SpiInit( &SX1272.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1272IoInit( );
#elif defined( SX1276MB1LAS ) || defined( SX1276MB1MAS ) || defined( SX1276UNDEFINED )
    SpiInit( &SX1276.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1276IoInit( );
#endif

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
#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    SpiDeInit( &SX126x.Spi );
    SX126xIoDeInit( );
#elif defined( SX1272MB2DAS)
    SpiDeInit( &SX1272.Spi );
    SX1272IoDeInit( );
#elif defined( SX1276MB1LAS ) || defined( SX1276MB1MAS ) || defined( SX1276UNDEFINED )
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );
#endif
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

uint16_t BoardBatteryMeasureVolage( void )
{
    return 0;
}

uint32_t BoardGetBatteryVoltage( void )
{
    return 0;
}

uint8_t BoardGetBatteryLevel( void )
{
    return 0;
}

static void BoardUnusedIoInit( void )
{
    HAL_DBGMCU_EnableDBGSleepMode( );
    HAL_DBGMCU_EnableDBGStopMode( );
    HAL_DBGMCU_EnableDBGStandbyMode( );
}

void SystemClockConfig( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    __HAL_RCC_PWR_CLK_ENABLE( );

    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
    RCC_OscInitStruct.LSEState            = RCC_LSE_ON;
    RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM            = 1;
    RCC_OscInitStruct.PLL.PLLN            = 40;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV4;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
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
    if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 ) != HAL_OK )
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
      RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
      RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
      uint32_t pFLatency = 0;

    CRITICAL_SECTION_BEGIN( );

    // In case nvic had a pending IT, the arm doesn't enter stop mode
    // Hence the pll is not switched off and will cause HAL_RCC_OscConfig return 
    // an error
    if ( __HAL_RCC_GET_SYSCLK_SOURCE() != RCC_CFGR_SWS_PLL )
    {
        // Enable Power Control clock
        __HAL_RCC_PWR_CLK_ENABLE( );

        // Get the Oscillators configuration according to the internal RCC registers */
        HAL_RCC_GetOscConfig( &RCC_OscInitStruct );

        // Enable PLL
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
        {
            while( 1 );
        }
        
        /* Get the Clocks configuration according to the internal RCC registers */
        HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
        
        /* Select PLL as system clock source and keep HCLK, PCLK1 and PCLK2 clocks dividers as before */
        RCC_ClkInitStruct.ClockType     = RCC_CLOCKTYPE_SYSCLK;
        RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
        {
            while(1);
        }
    }
    else
    {
        // MCU did not enter stop mode beacuse NVIC had a pending IT
    }

    CRITICAL_SECTION_END( );
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

    // Initilizes the peripherals
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
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
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

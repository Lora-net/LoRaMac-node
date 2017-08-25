/*
 / _____)			 _				| |
( (____	_____ ____ _| |_ _____	____| |__
 \____ \| ___ |	(_	_) ___ |/ ___)	_ \
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
#ifndef UID_BASE
#define UID_BASE	((uint32_t)0x1FF80050U)	/*!< Unique device ID register base address for Cat.1 and Cat.2 devices */
#endif
#define		 ID1	( UID_BASE + 0x00U )
#define		 ID2	( UID_BASE + 0x04U )
#define		 ID3	( UID_BASE + 0x14U )

/*!
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

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
 * Flag used to indicate if board is powered from external power
 */
static bool IsExtPower = false;

/*!
 * UART1 FIFO buffers size
 */
#define UART1_FIFO_TX_SIZE	8
#define UART1_FIFO_RX_SIZE	256

uint8_t UartTxBuffer[UART1_FIFO_TX_SIZE];
uint8_t UartRxBuffer[UART1_FIFO_RX_SIZE];

/*!
 * UART2 FIFO buffers size
 */
#define UART2_FIFO_TX_SIZE	1056
#define UART2_FIFO_RX_SIZE	1056

uint8_t Uart2TxBuffer[UART2_FIFO_TX_SIZE];
uint8_t Uart2RxBuffer[UART2_FIFO_RX_SIZE];

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
	if ( IrqNestLevel == 0 )
	{
		__enable_irq( );
	}
}

void BoardInitPeriph( void )
{
    Gpio_t ioPin;

	(void)ioPin;

    // Init the GPIO pins
#ifdef IRQ_MPL3115
    GpioInit( &ioPin, IRQ_MPL3115, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    // Init temperature, pressure and altitude sensor
    MPL3115Init( );
#endif

#ifdef IRQ_MAG3110
    GpioInit( &ioPin, IRQ_MAG3110, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    // Init magnetometer
    MAG3110Init( );
#endif

#ifdef GPS_POWER_ON
    GpioInit( &ioPin, GPS_POWER_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    // Init GPS
    GpsInit( );
#endif

#ifdef IRQ_1_MMA8451
    GpioInit( &ioPin, IRQ_1_MMA8451, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    // Init accelerometer
    MMA8451Init( );
#endif

#ifdef LED_1
    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioWrite( &Led1, 0 );
#endif

#ifdef LED_2
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioWrite( &Led2, 0 );
#endif

#ifdef LED_3
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioWrite( &Led3, 0 );
#endif
}

void BoardInitMcu( void )
{
	Gpio_t ioPin;
	(void)ioPin;

	if ( McuInitialized == false )
	{
#if defined( USE_BOOTLOADER )
		// Set the Vector Table base location at 0x3000
		SCB->VTOR = FLASH_BASE | 0x3000;
#endif
		HAL_Init( );

		SystemClockConfig();

		GpioInit( &ioPin, UART_RX, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
		if( GpioRead( &ioPin ) == 1 )   // Debug Mode, USART1 connected
		{
			IsExtPower = true;
			/*
			FifoInit( &Uart1.FifoTx, UartTxBuffer, UART1_FIFO_TX_SIZE );
			FifoInit( &Uart1.FifoRx, UartRxBuffer, UART1_FIFO_RX_SIZE );
			UartInit( &Uart1, UART_1, UART_TX, UART_RX );
			UartConfig( &Uart1, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
			*/
		}
		else
		{
			IsExtPower = false;
			/*
			UartDeInit( &Uart1 );
			*/
		}

		RtcInit( );

		BoardUnusedIoInit( );

        I2cInit( &I2c, I2C_SCL, I2C_SDA );
	}
	else
	{
		SystemClockReConfig();
	}

#ifdef BAT_LEVEL_PIN
	AdcInit( &Adc, BAT_LEVEL_PIN );
#endif

	SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
	SX1276IoInit( );

	if ( McuInitialized == false )
	{
		McuInitialized = true;
		if ( GetBoardPowerSource( ) == BATTERY_POWER )
		{
			CalibrateSystemWakeupTime( );
		}
	}
}

void BoardDeInitMcu( void )
{
	AdcDeInit( &Adc );

	SpiDeInit( &SX1276.Spi );
	SX1276IoDeInit( );

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

void BoardGetDevEUI( uint8_t *id )
{
    uint32_t *pDevEuiHWord = ( uint32_t* )&id[4];

    if( *pDevEuiHWord == 0 )
    {
        *pDevEuiHWord = BoardGetRandomSeed( );
    }
}

/*!
 * Factory power supply
 */
#define FACTORY_POWER_SUPPLY						3300 // mV

/*!
 * VREF calibration value
 */
#define VREFINT_CAL								 ( *( uint16_t* )0x1FF80078 )

/*!
 * ADC maximum value
 */
#define ADC_MAX_VALUE								4095

/*!
 * Battery thresholds
 */
#define BATTERY_MAX_LEVEL							4150 // mV
#define BATTERY_MIN_LEVEL							3200 // mV
#define BATTERY_SHUTDOWN_LEVEL						3100 // mV

static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

uint16_t BoardBatteryMeasureVolage( void )
{
	uint16_t batteryVoltage = 0;

#ifdef BAT_LEVEL_CHANNEL
	uint16_t vdd = 0;
	uint16_t vref = VREFINT_CAL;
	uint16_t vdiv = 0;

	vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL );
	// vref = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT );

	vdd = ( float )FACTORY_POWER_SUPPLY * ( float )VREFINT_CAL / ( float )vref;
	batteryVoltage = vdd * ( ( float )vdiv / ( float )ADC_MAX_VALUE );

	//								vDiv
	// Divider bridge	VBAT <-> 470k -<--|-->- 470k <-> GND => vBat = 2 * vDiv
	batteryVoltage = 2 * batteryVoltage;
#endif

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

	if ( GetBoardPowerSource( ) == USB_POWER )
	{
		batteryLevel = 0;
	}
	else
	{
		if ( BatteryVoltage >= BATTERY_MAX_LEVEL )
		{
			batteryLevel = 254;
		}
		else if ( ( BatteryVoltage > BATTERY_MIN_LEVEL ) && ( BatteryVoltage < BATTERY_MAX_LEVEL ) )
		{
			batteryLevel = ( ( 253 * ( BatteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
		}
		else if ( ( BatteryVoltage > BATTERY_SHUTDOWN_LEVEL ) && ( BatteryVoltage <= BATTERY_MIN_LEVEL ) )
		{
			batteryLevel = 1;
		}
		else // if ( BatteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
		{
			batteryLevel = 255;
		}
	}
	return batteryLevel;
}

static void BoardUnusedIoInit( void )
{
//	Gpio_t ioPin;

#if defined( USE_DEBUGGER )
	HAL_DBGMCU_EnableDBGStopMode( );
	HAL_DBGMCU_EnableDBGSleepMode( );
	HAL_DBGMCU_EnableDBGStandbyMode( );
	__HAL_DBGMCU_FREEZE_RTC();
	__HAL_DBGMCU_FREEZE_WWDG();
	__HAL_DBGMCU_FREEZE_IWDG();
	__HAL_DBGMCU_FREEZE_I2C1_TIMEOUT();
	__HAL_DBGMCU_FREEZE_I2C2_TIMEOUT();
#else
	HAL_DBGMCU_DisableDBGSleepMode( );
	HAL_DBGMCU_DisableDBGStopMode( );
	HAL_DBGMCU_DisableDBGStandbyMode( );

	GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

//    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

#ifdef BAT_LEVEL_PIN
    GpioInit( &ioPin, BAT_LEVEL_PIN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void SystemClockConfig( void )
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	__HAL_RCC_PWR_CLK_ENABLE( );

	__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
	if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
									RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 ) != HAL_OK )
	{
		assert_param( FAIL );
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if ( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
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
	if ( SystemWakeupTimeCalibrated == false )
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

	/* Enable HSI */
	__HAL_RCC_HSI_ENABLE();

	/* Wait till HSI is ready */
	while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSIRDY ) == RESET )
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
    if ( IsExtPower )
		return USB_POWER;
	return BATTERY_POWER;
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
	set to 'Yes') calls __io_putchar() */
int __io_putchar( int c )
#else /* __GNUC__ */
int fputc( int c, FILE *stream )
#endif
{
	while( UartMcuPutChar( &Uart1, c ) != 0 )
		;
	return c;
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name	: assert_failed
 * Description	: Reports the name of the source file and the source line number
 *					where the assert_param error has occurred.
 * Input			: - file: pointer to the source file name
 *					- line: assert_param error line source number
 * Output		 : None
 * Return		 : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %u\r\n", file, line) */

    printf( "Wrong parameters value: file %s on line %u\r\n", ( const char* )file, line );
	/* Infinite loop */
	while( 1 )
	{
	}
}
#endif

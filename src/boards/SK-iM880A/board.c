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
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

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
        // We use IRQ priority group 4 for the entire project
        // When setting the IRQ, only the preemption priority is used
        NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

        // Disable Systick
        SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off 
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag

        AdcInit( &Adc, POTI );

        SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
        SX1272IoInit( );

#if( LOW_POWER_MODE_ENABLE )
        TimerSetLowPowerEnable( true );
#else
        TimerSetLowPowerEnable( false );
#endif
        BoardUnusedIoInit( );

        if( TimerGetLowPowerEnable( ) == true )
        {
            RtcInit( );
        }
        else
        {
            TimerHwInit( );
        }
        McuInitialized = true;
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
    
    McuInitialized = false;
}

void BoardGetUniqueId( uint8_t *id )
{
    id[0] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[1] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[2] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[3] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[4] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[5] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[6] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[7] = ( ( *( uint32_t* )ID2 ) );
}

uint8_t BoardMeasurePotiLevel( void ) 
{
    uint8_t potiLevel = 0;
    uint16_t MeasuredLevel = 0;
     
    // read the current potentiometer setting
    MeasuredLevel = AdcMcuRead( &Adc , ADC_Channel_3 );

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
    MeasuredLevel = AdcMcuRead( &Adc , ADC_Channel_17 );
   
    // We don't use the VREF from calibValues here.
    // calculate the Voltage in miliVolt
    milliVolt = ( uint32_t )PDDADC_VREF_BANDGAP * ( uint32_t )PDDADC_MAX_VALUE;
    milliVolt = milliVolt / ( uint32_t ) MeasuredLevel;

    return ( uint16_t ) milliVolt;
}

uint8_t BoardMeasureBatterieLevel( void ) 
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
#if !defined( USE_USB_CDC ) || !defined( USE_DEBUGGER )
    Gpio_t ioPin;
#endif

    /* USB */
#if !defined( USE_USB_CDC )
    GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
#if defined( USE_DEBUGGER )
    DBGMCU_Config( DBGMCU_SLEEP, ENABLE );
    DBGMCU_Config( DBGMCU_STOP, ENABLE);
    DBGMCU_Config( DBGMCU_STANDBY, ENABLE);
#else
    DBGMCU_Config( DBGMCU_SLEEP, DISABLE );
    DBGMCU_Config( DBGMCU_STOP, DISABLE );
    DBGMCU_Config( DBGMCU_STANDBY, DISABLE );
    
    GpioInit( &ioPin, JTAG_TMS, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TCK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TDI, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TDO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_NRST, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 ); 
#endif    
}

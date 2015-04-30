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
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

/*!
 * Hex coder selector GPIO pins objects
 */
Gpio_t Sel1;
Gpio_t Sel2;
Gpio_t Sel3;
Gpio_t Sel4;
    
#if defined( USE_DEBUG_PINS )
Gpio_t DbgPin1;
Gpio_t DbgPin2;
Gpio_t DbgPin3;
Gpio_t DbgPin4;
#endif

I2c_t I2c;

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

volatile uint8_t Led3Status = 1;

void BoardInitPeriph( void )
{
    GpioInit( &Sel1, SEL_1, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel2, SEL_2, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel3, SEL_3, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel4, SEL_4, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, Led3Status );

    // Init temperature, pressure and altitude sensor
    MPL3115Init( );
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

        I2cInit( &I2c, I2C_SCL, I2C_SDA );

        SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
        SX1276IoInit( );

#if defined( USE_DEBUG_PINS )
        GpioInit( &DbgPin1, J1_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin2, J1_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin3, J1_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin4, J1_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
        BoardInitPeriph( );

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

    I2cDeInit( &I2c );
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );

    GpioInit( &Led1, LED_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led2, LED_2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    Led3Status = GpioRead( &Led3 );
    if( Led3Status == 1 )
    {
        GpioInit( &Led3, LED_3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    }

    GpioInit( &Sel1, SEL_1, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel2, SEL_2, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel3, SEL_3, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &Sel4, SEL_4, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

#if ( defined( USE_DEBUG_PINS ) && !defined( LOW_POWER_MODE_ENABLE ) )
    GpioInit( &DbgPin1, J1_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPin2, J1_2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPin3, J1_3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPin4, J1_4, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

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

uint8_t BoardMeasureBatterieLevel( void ) 
{
    uint8_t batteryLevel = 0;
    uint8_t i, result = 0;
    uint32_t tmpreg = 0;
    
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );
    
    PWR_PVDCmd( ENABLE ); // activate the voltage reference
    DelayMs( 1 );
    
    tmpreg = PWR->CR; // get the CR register for a read-modify-write
    
    for( i = 0; i <= 6; i++ ) 
    {
        PWR->CR = ( ( tmpreg & 0xFFFFFF1F ) | ( i << 5 ) ); // set PVD level from 0 to 6
        DelayMs( 1 );
        if( PWR_GetFlagStatus( PWR_FLAG_PVDO ) == 1 )
        {
            result |= 0x01 << i; // use 'result' as a bit array
        }
    }
    PWR_PVDCmd( DISABLE ); // shut down voltage reference
    
    switch( result ) { // transcribe result and detect invalid codes
        case 0x7F: 
            batteryLevel = 0x1F;    // Vbat < 1.9V
            break;
        case 0x7E: 
            batteryLevel = 0x3F;    // 1.9 < Vbat < 2.1
            break;
        case 0x7C: 
            batteryLevel = 0x5F;    // 2.1 < Vbat < 2.3
            break;
        case 0x78: 
            batteryLevel = 0x7F;    // 2.3 < Vbat < 2.5
            break;
        case 0x70: 
            batteryLevel = 0x9F;    // 2.5 < Vbat < 2.7
            break;
        case 0x60: 
            batteryLevel = 0xBF;    // 2.7 < Vbat < 2.9
            break;
        case 0x40: 
            batteryLevel = 0xDF;    // 2.9 < Vbat < 3.1
            break;
        case 0x00: 
            batteryLevel = 0xFE;    // Vbat > 3.1V
            break;
        default:   
            batteryLevel = 0xFF;    // Fail
            break;
    }
    return batteryLevel;
}

static void BoardUnusedIoInit( void )
{
    Gpio_t ioPin;

    /* Non Connected pin */
    GpioInit( &ioPin, NC_1, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_2, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_3, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_4, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

    /* External Connector J5 */
#if !defined( USE_DEBUG_PINS )
    GpioInit( &ioPin, J1_1, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, J1_2, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, J1_3, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, J1_4, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
#endif
   
    /* External Connector J2 */
    GpioInit( &ioPin, J2_2, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, J2_3, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 ); 
    
    /* SD Card */
    GpioInit( &ioPin, NC_7, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_8, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_9, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_10, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_11, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_12, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, NC_13, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    
    /* USB */
#if !defined( USE_USB_CDC )
    GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 ); 
#endif
    
    /* BOOT1 pin */
    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 ); 
     
#if defined( USE_DEBUGGER )
    DBGMCU_Config( DBGMCU_SLEEP, ENABLE );
    DBGMCU_Config( DBGMCU_STOP, ENABLE);
    DBGMCU_Config( DBGMCU_STANDBY, ENABLE);
#else
    DBGMCU_Config( DBGMCU_SLEEP, DISABLE );
    DBGMCU_Config( DBGMCU_STOP, DISABLE );
    DBGMCU_Config( DBGMCU_STANDBY, DISABLE );
    
    GpioInit( &ioPin, JTAG_TMS, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TCK, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TDI, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_TDO, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, JTAG_NRST, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 ); 
#endif    
}

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
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

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
        NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x3000 );
#endif
        // We use IRQ priority group 4 for the entire project
        // When setting the IRQ, only the preemption priority is used
        NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

        // Disable Systick
        SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off 
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag

        I2cInit( &I2c, I2C_SCL, I2C_SDA );
        AdcInit( &Adc, BAT_LEVEL );

        SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
        SX1272IoInit( );

#if defined( USE_DEBUG_PINS )
        GpioInit( &DbgPin1, CON_EXT_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin2, CON_EXT_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin3, CON_EXT_7, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &DbgPin4, CON_EXT_9, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

#if defined( USE_USB_CDC )
        {
            Gpio_t usbDM;

            GpioInit( &usbDM, USB_DM, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );

            if( GpioRead( &usbDM ) == 0 )
            {
                TimerSetLowPowerEnable( false );
                UsbMcuInit( );
                UartInit( &UartUsb, UART_USB_CDC, NC, NC );
                UartConfig( &UartUsb, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
            }
            else
            {
                TimerSetLowPowerEnable( true );
                GpioInit( &usbDM, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
            }
        }
#elif( LOW_POWER_MODE_ENABLE )
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
    SpiDeInit( &SX1272.Spi );
    SX1272IoDeInit( );

#if ( defined( USE_DEBUG_PINS ) && !defined( LOW_POWER_MODE_ENABLE ) )
    GpioInit( &DbgPin1, CON_EXT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPin2, CON_EXT_3, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPin3, CON_EXT_7, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPin4, CON_EXT_9, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
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

    /* USB */
#if !defined( USE_USB_CDC )
    GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    GpioInit( &ioPin, BOOT_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
     
    GpioInit( &ioPin, CON_EXT_8, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, BAT_LEVEL, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
               
    GpioInit( &ioPin, PIN_PB6, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ioPin, WKUP1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
     
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

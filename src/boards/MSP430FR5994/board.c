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
#include "msp430.h"
#include "utilities.h"
#include "gpio_sys.h"
//#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
//#include "timer.h"
//#include "sysIrqHandlers.h"
#include "board-config.h"
#include "lpm-board.h"
#include "cs.h"
//#include "rtc-board.h"

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    #include "sx126x-board.h"
#elif defined( LR1110MB1XXS )
    #include "lr1110-board.h"
#elif defined( SX1272MB2DAS)
    #include "sx1272-board.h"
#endif

#include "board.h"

/*!
 * Unique Devices IDs register set (  )
 */
#define         BOARD_ID1                                 ( 0x1FF800D0 )
#define         BOARD_ID2                                 ( 0x1FF800D4 )
#define         BOARD_ID3                                 ( 0x1FF800E4 )

/*!
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;

/*
 * MCU objects
 */
//Adc_t  Adc;
Uart_t Uart2;

#if defined( LR1110MB1XXS )
    extern lr1110_t LR1110;
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
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );

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
#define UART2_FIFO_TX_SIZE                                5
#define UART2_FIFO_RX_SIZE                                128

uint8_t Uart2TxBuffer[UART2_FIFO_TX_SIZE];
uint8_t Uart2RxBuffer[UART2_FIFO_RX_SIZE];

void BoardCriticalSectionBegin( uint32_t *mask )
{
    __disable_interrupt( );
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    __enable_interrupt();
}

void BoardInitPeriph( void )
{
    //TODO?
}

void BoardInitMcu( void )
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    if( McuInitialized == false )
    {
        // LEDs
        GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

        //Required for clock config --> TODO: update GpioInit and use it to configure these pins accordingly
        PJOUT = 0;
        PJDIR = 0xFF;
        PJSEL0 = BIT4 | BIT5;               // For XT1

        // Disable the GPIO power-on default high-impedance mode to activate
        // previously configured port settings
        PM5CTL0 &= ~LOCKLPM5;

        SystemClockConfig( );

        UsbIsConnected = true;

        FifoInit( &Uart2.FifoTx, Uart2TxBuffer, UART2_FIFO_TX_SIZE );
        FifoInit( &Uart2.FifoRx, Uart2RxBuffer, UART2_FIFO_RX_SIZE );
        // Configure your terminal for 8 Bits data (7 data bit + 1 parity bit), no parity and no flow ctrl
        UartInit( &Uart2, UART_2, UART_TX, UART_RX );
        UartConfig( &Uart2, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );

        //RtcInit( );

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

  //  AdcInit( &Adc, NC );  // Just initialize ADC
/*
#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    SpiInit( &SX126x.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX126xIoInit( );
#elif defined( LR1110MB1XXS )
    SpiInit( &LR1110.spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    lr1110_board_init_io( &LR1110 );
#elif defined( SX1272MB2DAS )
    SpiInit( &SX1272.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1272IoInit( );
#elif defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    SpiInit( &SX1276.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1276IoInit( );
#endif*/

 /*   if( McuInitialized == false )
    {
        McuInitialized = true;
#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
        SX126xIoDbgInit( );
        // WARNING: If necessary the TCXO control is initialized by SX126xInit function.
#elif defined( LR1110MB1XXS )
        lr1110_board_init_dbg_io( &LR1110 );
        // WARNING: If necessary the TCXO control is initialized by SX126xInit function.
#elif defined( SX1272MB2DAS )
        SX1272IoDbgInit( );
        SX1272IoTcxoInit( );
#elif defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
        SX1276IoDbgInit( );
        SX1276IoTcxoInit( );
#endif
    }*/
}

void BoardResetMcu( void )
{
    //Restart system
    WDTCTL = 0xDEAD;
}

void BoardDeInitMcu( void )
{
    //AdcDeInit( &Adc );

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    SpiDeInit( &SX126x.Spi );
    SX126xIoDeInit( );
#elif defined( LR1110MB1XXS )
    SpiDeInit( &LR1110.spi );
    lr1110_board_deinit_io( &LR1110 );
#elif defined( SX1272MB2DAS )
    SpiDeInit( &SX1272.Spi );
    SX1272IoDeInit( );
#elif defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );
#endif
}

uint32_t BoardGetRandomSeed( void )
{
    return ( ( *( uint32_t* )BOARD_ID1 ) ^ ( *( uint32_t* )BOARD_ID2 ) ^ ( *( uint32_t* )BOARD_ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
    // id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    // id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    // id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    // id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    // id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    // id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    // id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    // id[0] = ( ( *( uint32_t* )ID2 ) );

    id[7] = 0;
    id[6] = 1;
    id[5] = 2;
    id[4] = 3;
    id[3] = 4;
    id[2] = 5;
    id[1] = 6;
    id[0] = 7;
}

uint16_t BoardBatteryMeasureVoltage( void )
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

int16_t BoardGetTemperature( void )
{
    return 0;
}

static void BoardUnusedIoInit( void )
{
    
}

//Configure to 8 MHz
void SystemClockConfig( void )
{
   // Set DCO frequency to 8 MHz
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
    //Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768, 0);
    //Set ACLK=LFXT
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //Start XT1 with no time out
    CS_turnOnLFXT(CS_LFXT_DRIVE_3);
}

void SystemClockReConfig( void )
{
    
}

void SysTick_Handler( void )
{

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

void BoardLowPowerHandler( void )
{
    __disable_interrupt( );
    /*!
     * If an interrupt has occurred after __disable_irq( ), it is kept pending 
     * and cortex will not enter low power anyway
     */

    LpmEnterLowPower( );

    __enable_interrupt( );
}

#if 1
/*
 * Function to be used by stdout for printf etc
 */
int write( int fd, const void *buf, unsigned int count )
{
    UartPutBuffer( &Uart2, ( uint8_t* )buf, ( uint16_t )count );
    return count;
}

/*
 * Function to be used by stdin for scanf etc
 */
int read( int fd, const void *buf, unsigned int count )
{
    unsigned int bytesRead = 0;
    while( UartGetBuffer( &Uart2, ( uint8_t* )buf, count, ( uint16_t* )&bytesRead ) != 0 ){ };
    // Echo back the character
    while( UartPutBuffer( &Uart2, ( uint8_t* )buf, ( uint16_t )bytesRead ) != 0 ){ };
    return bytesRead;
}

#endif

/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
#if 0
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %lu\n", file, line) */

    printf( "Wrong parameters value: file %s on line %lu\n", ( const char* )file, line );
    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif

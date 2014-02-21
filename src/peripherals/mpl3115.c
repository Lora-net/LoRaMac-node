/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Driver for the MPL3115 Temperature, pressure and altitude sensor

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "mpl3115.h"

static uint8_t I2cDeviceAddr = 0;

static bool mpl3115Initialized = false;

/******************************************************************************
* Global variables
******************************************************************************/

/*
 * Hold the last pressure measured
 */
float Pressure;

/*
 * Hold the last altitude measured
 */
float Altitude;

/*
 * Hold the last Temperature measured
 */
float Temperature;

/*!
 * \brief Writes a byte at specified address in the device
 *
 * \param [IN]:    addr
 * \param [IN]:    data
 * \retval status [SUCCESS, FAIL]
 */
uint8_t mpl3115Write( uint8_t addr, uint8_t data );

/*!
 * \brief Writes a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [IN]: data
 * \param [IN]: size
 * \retval status [SUCCESS, FAIL]
 */
uint8_t mpl3115WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Reads a byte at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \retval status [SUCCESS, FAIL]
 */
uint8_t mpl3115Read( uint8_t addr, uint8_t *data );

/*!
 * \brief Reads a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \param [IN]: size
 * \retval status [SUCCESS, FAIL]
 */
uint8_t mpl3115ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Sets the I2C device slave address
 *
 * \param [IN]: addr
 */
void mpl3115SetDeviceAddr( uint8_t addr );

/*!
 * \brief Gets the I2C device slave address
 *
 * \retval: addr Current device slave address
 */
uint8_t mpl3115GetDeviceAddr( void );

/*!
 * \brief Set the device in Marometer Mode
 */
void mpl3115SetModeBarometer( void );

/*!
 * \brief Set the device in Altimeter Mode
 */
void mpl3115SetModeAltimeter( void );

/*!
 * \brief Set the device in Standby
 */
void mpl3115SetModeStandby( void );

/*!
 * \brief Set the device in Measure Mode
 */
void mpl3115SetModeActive( void );

/*!
 * \brief Toggle the OST bit causing the sensor to immediately take another reading
 */
void mpl3115ToggleOneShot( void );


uint8_t mpl3115Init( void )
{
    uint8_t regVal = 0;

    mpl3115SetDeviceAddr( MPL3115A_I2C_ADDRESS );

    if( mpl3115Initialized == false )
    {   
        mpl3115Initialized = true;
        
        mpl3115Read( MPL3115_ID, &regVal );
        if( regVal != 0xC4 )
        {
            return FAIL;
        }
    
        mpl3115Reset( );
    
        do
        {   // Wait for the RST bit to clear 
            mpl3115Read( CTRL_REG1, &regVal );
        }while( regVal );
    
        mpl3115Write( PT_DATA_CFG_REG, 0x07 );        // Enable data flags 
        mpl3115Write( CTRL_REG3, 0x11 );            // Open drain, active low interrupts 
        mpl3115Write( CTRL_REG4, 0x80 );            // Enable DRDY interrupt 
        mpl3115Write( CTRL_REG5, 0x00 );            // DRDY interrupt routed to INT2 - PTD3 
        mpl3115Write( CTRL_REG1, 0xA9 );            // Active altitude mode, OSR = 32    

        mpl3115Write( OFF_H_REG, 0xB0 );            // Altitude data offset
    
        mpl3115SetModeActive( );
    }
    return SUCCESS;
}

uint8_t mpl3115Reset( )
{
    // Reset all registers to POR values
    if( mpl3115Write( CTRL_REG1, 0x04 ) == SUCCESS )
    {
        return SUCCESS;
    }
    return FAIL;
}

uint8_t mpl3115Write( uint8_t addr, uint8_t data )
{
    return mpl3115WriteBuffer( addr, &data, 1 );
}

uint8_t mpl3115WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t mpl3115Read( uint8_t addr, uint8_t *data )
{
    return mpl3115ReadBuffer( addr, data, 1 );
}

uint8_t mpl3115ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void mpl3115SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t mpl3115GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}

float mpl3115ReadAltitude( void )
{
    uint8_t val = 0;
    uint8_t msb = 0, csb = 0, lsb = 0;
    float decimal = 0;
    uint32_t counter = 0;
  
    mpl3115SetModeAltimeter( );
    mpl3115ToggleOneShot( );

    while( ( val & 0x04 ) != 0x04 )
    {    
        mpl3115Read( STATUS_REG, &val );
        counter++;
        if( counter > 0xA000 )
        {    
            counter = 0;
            mpl3115Initialized = false;
            mpl3115Init( );
            mpl3115SetModeAltimeter( );
            mpl3115ToggleOneShot( );

            while( ( val & 0x04 ) != 0x04 )
            {  
                mpl3115Read( STATUS_REG, &val );
                counter++;
                if( counter > 0xA000 )
                { 
                    return( 0 ); //Error out after max of 512ms for a read
                }
            }
        }
    }

    mpl3115Read( OUT_P_MSB_REG, &msb );        // High byte of integer part of altitude,  
    mpl3115Read( OUT_P_CSB_REG, &csb );        // Low byte of integer part of altitude 
    mpl3115Read( OUT_P_LSB_REG, &lsb );        // Decimal part of altitude in bits 7-4
    
    decimal = ( ( float )( lsb >> 4 ) ) / 16.0;
    Altitude = ( float )( ( msb << 8 ) | csb ) + decimal;

    return( Altitude );
}

float mpl3115ReadPressure( void )
{
    uint8_t val = 0;
    uint8_t msb = 0, csb = 0, lsb = 0;
    float decimal = 0;
    uint32_t counter = 0;

    mpl3115SetModeBarometer( );
    mpl3115ToggleOneShot( );

    while( ( val & 0x04 ) != 0x04 )
    {    
        mpl3115Read( STATUS_REG, &val );
        counter++;
        if( counter > 0xA000  )
        {    
            counter = 0;
            mpl3115Initialized = false;
            mpl3115Init( );
            mpl3115SetModeBarometer( );
            mpl3115ToggleOneShot( );
            while( ( val & 0x04 ) != 0x04 )
            {  
                mpl3115Read( STATUS_REG, &val );
                counter++;
                if( counter > 0xA000 )
                { 
                    return( 0 ); //Error out after max of 512ms for a read
                }
            }
        }
    }

    mpl3115Read( OUT_P_MSB_REG, &msb );        // High byte of integer part of pressure,  
    mpl3115Read( OUT_P_CSB_REG, &csb );        // Low byte of integer part of pressure 
    mpl3115Read( OUT_P_LSB_REG, &lsb );        // Decimal part of pressure in bits 7-4

    Pressure = ( float )( ( msb << 16 | csb << 8 | lsb ) >> 6 );
    lsb &= 0x30; //Bits 5/4 represent the fractional component
    lsb >>= 4; //Get it right aligned
    
    decimal = ( ( float )lsb ) / 4.0;

    Pressure = Pressure + decimal;

    mpl3115ToggleOneShot( );

    return( Pressure );
}

float mpl3115ReadTemperature( void )
{
    bool negSign = false;
    uint8_t val = 0;
    uint8_t msb = 0, lsb = 0;
    uint32_t counter = 0;

    mpl3115ToggleOneShot( );

    while( ( val & 0x02 ) != 0x02 )
    {    
        mpl3115Read( STATUS_REG, &val );
        counter++;
        if( counter > 0xA000 ) 
        {    
            return( 0 ); //Error out after max of 512ms for a read
        }
    }

    mpl3115Read( OUT_T_MSB_REG, &msb );        // Integer part of temperature 
    mpl3115Read( OUT_T_LSB_REG, &lsb );        // Decimal part of temperature in bits 7-4

    if( msb > 0x7F )
    {
        val = ~( ( msb << 8 ) + lsb ) + 1;  //2’s complement
        msb = val >> 8;
        lsb = val & 0x00F0; 
        negSign = true;
    }

    if( negSign == true )
    {
        Temperature = 0 - ( msb + ( float )( ( lsb >> 4 ) / 16.0 ) );
    }
    else
    {
        Temperature = msb + ( float )( ( lsb >> 4 ) / 16.0 );
    }

    mpl3115ToggleOneShot( );

    return( Temperature );
}


void mpl3115ToggleOneShot( void )
{
    uint8_t val = 0;

    mpl3115SetModeStandby( );

    mpl3115Read( CTRL_REG1, &val );
    val &= ~(0x02);         //Clear OST bit
    mpl3115Write( CTRL_REG1, val );

    mpl3115Read( CTRL_REG1, &val );
    val |= 0x02;            //Set OST bit
    mpl3115Write( CTRL_REG1, val );

    mpl3115SetModeActive( );
}

void mpl3115SetModeBarometer( void )
{
    uint8_t val = 0;

    mpl3115SetModeStandby( );

    mpl3115Read( CTRL_REG1, &val );
    val &= ~( 0x80 );           //Clear ALT bit
    mpl3115Write( CTRL_REG1, val );

    mpl3115SetModeActive( );
}

void mpl3115SetModeAltimeter( void )
{
    uint8_t val = 0;

    mpl3115SetModeStandby( );

    mpl3115Read( CTRL_REG1, &val );
    val |= 0x80;                //Set ALT bit
    mpl3115Write( CTRL_REG1, val );

    mpl3115SetModeActive( );
}

void mpl3115SetModeStandby( void )
{
    uint8_t val = 0;
    mpl3115Read( CTRL_REG1, &val );
    val &= ~( 0x01 );         //Clear SBYB bit for Standby mode
    mpl3115Write( CTRL_REG1, val );
}

void mpl3115SetModeActive( void )
{
    uint8_t val = 0;
    mpl3115Read( CTRL_REG1, &val );
    val |= 0x01;                  //Set SBYB bit for Active mode
    mpl3115Write( CTRL_REG1, val );
}

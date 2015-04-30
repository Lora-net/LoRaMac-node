/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Driver for the MPL3115 Temperature, pressure and altitude sensor

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "mpl3115.h"

/*!
 * Holds the last measured pressure
 */
float Pressure;

/*!
 * Holds the last measured altitude
 */
float Altitude;

/*!
 * Holds the last measured temperature
 */
float Temperature;

static uint8_t I2cDeviceAddr = 0;
static bool MPL3115Initialized = false;

/*!
 * \brief Writes a byte at specified address in the device
 *
 * \param [IN]:    addr
 * \param [IN]:    data
 * \retval status [SUCCESS, FAIL]
 */
uint8_t MPL3115Write( uint8_t addr, uint8_t data );

/*!
 * \brief Writes a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [IN]: data
 * \param [IN]: size
 * \retval status [SUCCESS, FAIL]
 */
uint8_t MPL3115WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Reads a byte at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \retval status [SUCCESS, FAIL]
 */
uint8_t MPL3115Read( uint8_t addr, uint8_t *data );

/*!
 * \brief Reads a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \param [IN]: size
 * \retval status [SUCCESS, FAIL]
 */
uint8_t MPL3115ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Sets the I2C device slave address
 *
 * \param [IN]: addr
 */
void MPL3115SetDeviceAddr( uint8_t addr );

/*!
 * \brief Gets the I2C device slave address
 *
 * \retval: addr Current device slave address
 */
uint8_t MPL3115GetDeviceAddr( void );

/*!
 * \brief Sets the device in barometer Mode
 */
void MPL3115SetModeBarometer( void );

/*!
 * \brief Sets the device in altimeter Mode
 */
void MPL3115SetModeAltimeter( void );

/*!
 * \brief Sets the device in standby
 */
void MPL3115SetModeStandby( void );

/*!
 * \brief Sets the device in active Mode
 */
void MPL3115SetModeActive( void );

/*!
 * \brief Toggles the OST bit causing the sensor to immediately take another
 *        reading
 */
void MPL3115ToggleOneShot( void );


uint8_t MPL3115Init( void )
{
    uint8_t regVal = 0;

    MPL3115SetDeviceAddr( MPL3115A_I2C_ADDRESS );

    if( MPL3115Initialized == false )
    {          
        MPL3115Read( MPL3115_ID, &regVal );
        if( regVal != 0xC4 )
        {
            return FAIL;
        }
    
        MPL3115Reset( );
    
        do
        {   // Wait for the RST bit to clear 
            MPL3115Read( CTRL_REG1, &regVal );
        }while( regVal );
    
        MPL3115Write( PT_DATA_CFG_REG, 0x07 ); // Enable data flags 
        MPL3115Write( CTRL_REG3, 0x11 );       // Open drain, active low interrupts 
        MPL3115Write( CTRL_REG4, 0x80 );       // Enable DRDY interrupt 
        MPL3115Write( CTRL_REG5, 0x00 );       // DRDY interrupt routed to INT2 - PTD3 
        MPL3115Write( CTRL_REG1, 0xA9 );       // Active altitude mode, OSR = 32    
                                               
        MPL3115Write( OFF_H_REG, 0xB0 );       // Altitude data offset
    
        MPL3115SetModeActive( );
    
        MPL3115Initialized = true;
    }
    return SUCCESS;
}

uint8_t MPL3115Reset( )
{
    // Reset all registers to POR values
    if( MPL3115Write( CTRL_REG1, 0x04 ) == SUCCESS )
    {
        return SUCCESS;
    }
    return FAIL;
}

uint8_t MPL3115Write( uint8_t addr, uint8_t data )
{
    return MPL3115WriteBuffer( addr, &data, 1 );
}

uint8_t MPL3115WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cWriteBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

uint8_t MPL3115Read( uint8_t addr, uint8_t *data )
{
    return MPL3115ReadBuffer( addr, data, 1 );
}

uint8_t MPL3115ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size )
{
    return I2cReadBuffer( &I2c, I2cDeviceAddr << 1, addr, data, size );
}

void MPL3115SetDeviceAddr( uint8_t addr )
{
    I2cDeviceAddr = addr;
}

uint8_t MPL3115GetDeviceAddr( void )
{
    return I2cDeviceAddr;
}

float MPL3115ReadAltitude( void )
{
    uint8_t counter = 0;
    uint8_t val = 0;
    uint8_t msb = 0, csb = 0, lsb = 0;
    float decimal = 0;

    if( MPL3115Initialized == false )
    {
        return 0;
    }

    MPL3115SetModeAltimeter( );
    MPL3115ToggleOneShot( );

    while( ( val & 0x04 ) != 0x04 )
    {    
        MPL3115Read( STATUS_REG, &val );
        DelayMs( 10 );
        counter++;
    
        if( counter > 20 )
        {    
            MPL3115Initialized = false;
            MPL3115Init( );
            MPL3115SetModeAltimeter( );
            MPL3115ToggleOneShot( );
            counter = 0;
            while( ( val & 0x04 ) != 0x04 )
            {  
                MPL3115Read( STATUS_REG, &val );
                DelayMs( 10 );
                counter++;
                if( counter > 20 )
                {
                    return( 0 ); //Error out after max of 512ms for a read
                }
            }
        }
    }

    MPL3115Read( OUT_P_MSB_REG, &msb ); // High byte of integer part of altitude,  
    MPL3115Read( OUT_P_CSB_REG, &csb ); // Low byte of integer part of altitude 
    MPL3115Read( OUT_P_LSB_REG, &lsb ); // Decimal part of altitude in bits 7-4
    
    decimal = ( ( float )( lsb >> 4 ) ) / 16.0;
    Altitude = ( float )( ( int16_t )( ( msb << 8 ) | csb ) ) + decimal;

    return( Altitude );
}

float MPL3115ReadPressure( void )
{
    uint8_t counter = 0;
    uint8_t val = 0;
    uint8_t msb = 0, csb = 0, lsb = 0;
    float decimal = 0;

    if( MPL3115Initialized == false )
    {
        return 0;
    }

    MPL3115SetModeBarometer( );
    MPL3115ToggleOneShot( );

    while( ( val & 0x04 ) != 0x04 )
    {    
        MPL3115Read( STATUS_REG, &val );
        DelayMs( 10 );
        counter++;
    
        if( counter > 20 )
        {      
            MPL3115Initialized = false;
            MPL3115Init( );
            MPL3115SetModeBarometer( );
            MPL3115ToggleOneShot( );
            counter = 0;
            while( ( val & 0x04 ) != 0x04 )
            {
                DelayMs( 10 );
                counter++;
    
                if( counter > 20 )
                {
                    return( 0 ); //Error out after max of 512ms for a read
                }
            }
                
        }
    }

    MPL3115Read( OUT_P_MSB_REG, &msb ); // High byte of integer part of pressure,  
    MPL3115Read( OUT_P_CSB_REG, &csb ); // Low byte of integer part of pressure 
    MPL3115Read( OUT_P_LSB_REG, &lsb ); // Decimal part of pressure in bits 7-4

    Pressure = ( float )( ( msb << 16 | csb << 8 | lsb ) >> 6 );
    lsb &= 0x30; //Bits 5/4 represent the fractional component
    lsb >>= 4; //Get it right aligned
    
    decimal = ( ( float )lsb ) / 4.0;

    Pressure = Pressure + decimal;

    MPL3115ToggleOneShot( );

    return( Pressure );
}

float MPL3115ReadTemperature( void )
{
    uint8_t counter = 0;
    bool negSign = false;
    uint8_t val = 0;
    uint8_t msb = 0, lsb = 0;

    if( MPL3115Initialized == false )
    {
        return 0;
    }

    MPL3115ToggleOneShot( );

    while( ( val & 0x02 ) != 0x02 )
    {    
        MPL3115Read( STATUS_REG, &val );
        DelayMs( 10 );
        counter++;
    
        if( counter > 20 )
        { 
            MPL3115Initialized = false;
            MPL3115Init( );
            MPL3115ToggleOneShot( );
            counter = 0;
            while( ( val & 0x02 ) != 0x02 )
            {
                MPL3115Read( STATUS_REG, &val );
                DelayMs( 10 );
                counter++;
            
                if( counter > 20 )
                { 
                    return( 0 ); //Error out after max of 512ms for a read
                }
            }
                
        }
    }

    MPL3115Read( OUT_T_MSB_REG, &msb ); // Integer part of temperature 
    MPL3115Read( OUT_T_LSB_REG, &lsb ); // Decimal part of temperature in bits 7-4

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

    MPL3115ToggleOneShot( );

    return( Temperature );
}


void MPL3115ToggleOneShot( void )
{
    uint8_t val = 0;

    MPL3115SetModeStandby( );

    MPL3115Read( CTRL_REG1, &val );
    val &= ~(0x02);         //Clear OST bit
    MPL3115Write( CTRL_REG1, val );

    MPL3115Read( CTRL_REG1, &val );
    val |= 0x02;            //Set OST bit
    MPL3115Write( CTRL_REG1, val );

    MPL3115SetModeActive( );
}

void MPL3115SetModeBarometer( void )
{
    uint8_t val = 0;

    MPL3115SetModeStandby( );

    MPL3115Read( CTRL_REG1, &val );
    val &= ~( 0x80 );           //Clear ALT bit
    MPL3115Write( CTRL_REG1, val );

    MPL3115SetModeActive( );
}

void MPL3115SetModeAltimeter( void )
{
    uint8_t val = 0;

    MPL3115SetModeStandby( );

    MPL3115Read( CTRL_REG1, &val );
    val |= 0x80;                //Set ALT bit
    MPL3115Write( CTRL_REG1, val );

    MPL3115SetModeActive( );
}

void MPL3115SetModeStandby( void )
{
    uint8_t val = 0;
    MPL3115Read( CTRL_REG1, &val );
    val &= ~( 0x01 );         //Clear SBYB bit for Standby mode
    MPL3115Write( CTRL_REG1, val );
}

void MPL3115SetModeActive( void )
{
    uint8_t val = 0;
    MPL3115Read( CTRL_REG1, &val );
    val |= 0x01;                  //Set SBYB bit for Active mode
    MPL3115Write( CTRL_REG1, val );
}

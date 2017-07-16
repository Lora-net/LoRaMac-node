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
 * I2C device address
 */
static uint8_t I2cDeviceAddr = 0;

/*!
 * Indicates if the MPL3115 is initialized or not
 */
static bool MPL3115Initialized = false;

/*!
 * Defines the barometric reading types
 */
typedef enum
{
    PRESSURE,
    ALTITUDE,
}BarometerReadingType_t;

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
        MPL3115Write( CTRL_REG1, RST );
        DelayMs( 50 );
        I2cResetBus( &I2c );

        // Check MPL3115 ID
        MPL3115Read( MPL3115_ID, &regVal );
        if( regVal != 0xC4 )
        {
            return FAIL;
        }

        MPL3115Write( PT_DATA_CFG_REG, DREM | PDEFE | TDEFE );      // Enable data ready flags for pressure and temperature )
        MPL3115Write( CTRL_REG1, ALT | OS_32 | SBYB );              // Set sensor to active state with oversampling ratio 128 (512 ms between samples)
        MPL3115Initialized = true;
    }
    return SUCCESS;
}

uint8_t MPL3115Reset( void )
{
    // Reset all registers to POR values
    if( MPL3115Write( CTRL_REG1, RST ) == SUCCESS )
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

static uint32_t MPL3115ReadBarometer( BarometerReadingType_t type )
{
    uint8_t counter = 0;
    uint32_t resultBits = 0;
    uint8_t status = 0;

    if( MPL3115Initialized == false )
    {
        return 0;
    }

    if( type == ALTITUDE )
    {
        MPL3115SetModeAltimeter( );
    }
    else
    {
        MPL3115SetModeBarometer( );
    }

    MPL3115ToggleOneShot( );

    while( ( status & PDR ) != PDR )
    {
        MPL3115Read( STATUS_REG, &status );
        DelayMs( 10 );
        counter++;

        if( counter > 20 )
        {
            MPL3115Initialized = false;
            MPL3115Init( );
            if( type == ALTITUDE )
            {
                MPL3115SetModeAltimeter( );
            }
            else
            {
                MPL3115SetModeBarometer( );
            }
            MPL3115ToggleOneShot( );
            counter = 0;

            while( ( status & PDR ) != PDR )
            {
                MPL3115Read( STATUS_REG, &status );
                DelayMs( 10 );
                counter++;

                if( counter > 20 )
                {
                    // Error out after max of 512 ms for a read
                    return 0;
                }
            }
        }
    }

    MPL3115ReadBuffer( OUT_P_MSB_REG, (uint8_t*)&resultBits, 3 );       //Read altitude data

    return resultBits;
}

int32_t MPL3115ReadAltitudeInMetersTimes16( void )
{
    /*resultBits is unsigned so right shift is defined*/
    uint32_t resultBits = MPL3115ReadBarometer( ALTITUDE ) & 0x00FFFFFF;
    //shift out unused 4 lsb's
    resultBits >>= 4;
    //if it was negative
    if(resultBits & 0x00080000)
    {
      //sign extend
      resultBits |= 0xFFF00000;
    }
    return ( int32_t )resultBits;
}

uint32_t MPL3115ReadPressurePascalsTimes4( void )
{
    uint32_t resultBits = MPL3115ReadBarometer( PRESSURE ) & 0x00FFFFFF;
    //shift out unused 4 lsb's
    return ( resultBits >> 4 );
}

int16_t MPL3115ReadTemperatureDegCTimes4( void )
{
    uint8_t counter = 0;
    /*resultBits is unsigned so right shift is defined*/
    uint16_t resultBits = 0;
    uint8_t status = 0;

    if( MPL3115Initialized == false )
    {
        return 0;
    }

    MPL3115ToggleOneShot( );

    while( ( status & TDR ) != TDR )
    {
        MPL3115Read( STATUS_REG, &status );
        DelayMs( 10 );
        counter++;

        if( counter > 20 )
        {
            MPL3115Initialized = false;
            MPL3115Init( );
            MPL3115ToggleOneShot( );
            counter = 0;

            while( ( status & TDR ) != TDR )
            {
                MPL3115Read( STATUS_REG, &status );
                DelayMs( 10 );
                counter++;

                if( counter > 20 )
                {
                    // Error out after max of 512 ms for a read
                    return 0;
                }
            }
        }
    }

    MPL3115ReadBuffer( OUT_T_MSB_REG, (uint8_t*)&resultBits, 2 );

    //shift out unused 4 lsb's
    resultBits >>= 4;
    //if it was negative
    if(resultBits & 0x0800)
    {
      //sign extend
      resultBits |= 0xF000;
    }

    MPL3115ToggleOneShot( );

    return( (int16_t) resultBits );
}

void MPL3115ToggleOneShot( void )
{
    uint8_t ctrlReg = 0;

    MPL3115SetModeStandby( );

    MPL3115Read( CTRL_REG1, &ctrlReg );           // Read current settings
    ctrlReg &= ~OST;                              // Clear OST bit
    MPL3115Write( CTRL_REG1, ctrlReg );

    MPL3115Read( CTRL_REG1, &ctrlReg );           // Read current settings to be safe
    ctrlReg |= OST;                               // Set OST bit
    MPL3115Write( CTRL_REG1, ctrlReg );

    MPL3115SetModeActive( );
}

void MPL3115SetModeBarometer( void )
{
    uint8_t ctrlReg = 0;

    MPL3115SetModeStandby( );

    MPL3115Read( CTRL_REG1, &ctrlReg );           // Read current settings
    ctrlReg &= ~ALT;                              // Set ALT bit to zero
    MPL3115Write( CTRL_REG1, ctrlReg );

    MPL3115SetModeActive( );
}

void MPL3115SetModeAltimeter( void )
{
    uint8_t ctrlReg = 0;

    MPL3115SetModeStandby( );

    MPL3115Read( CTRL_REG1, &ctrlReg );           // Read current settings
    ctrlReg |= ALT;                               // Set ALT bit to one
    MPL3115Write( CTRL_REG1, ctrlReg );

    MPL3115SetModeActive( );
}

void MPL3115SetModeStandby( void )
{
    uint8_t ctrlReg = 0;

    MPL3115Read( CTRL_REG1, &ctrlReg );
    ctrlReg &= ~SBYB;                             // Clear SBYB bit for Standby mode
    MPL3115Write( CTRL_REG1, ctrlReg );
}

void MPL3115SetModeActive( void )
{
    uint8_t ctrlReg = 0;

    MPL3115Read( CTRL_REG1, &ctrlReg );
    ctrlReg |= SBYB;                              // Set SBYB bit for Active mode
    MPL3115Write( CTRL_REG1, ctrlReg );
}

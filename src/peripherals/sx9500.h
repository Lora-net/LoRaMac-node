/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Driver for the SX9500 proximity sensor

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __SX9500_H__
#define __SX9500_H__

#define SX9500_I2C_ADDRESS                          0x28

uint8_t sx9500Init( void );

/*!
 * \brief Resets the device
 *
 * \retval status [OK, ERROR, UNSUPPORTED]
 */
uint8_t sx9500Reset( void );

/*!
 * \brief Writes a byte at specified address in the device
 *
 * \param [IN]:	addr
 * \param [IN]:	data
 * \retval status [OK, ERROR, UNSUPPORTED]
 */
uint8_t sx9500Write( uint8_t addr, uint8_t data );

/*!
 * \brief Writes a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [IN]: data
 * \param [IN]: size
 * \retval status [OK, ERROR, UNSUPPORTED]
 */
uint8_t sx9500WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Reads a byte at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \retval status [OK, ERROR, UNSUPPORTED]
 */
uint8_t sx9500Read( uint8_t addr, uint8_t *data );

/*!
 * \brief Reads a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \param [IN]: size
 * \retval status [OK, ERROR, UNSUPPORTED]
 */
uint8_t sx9500ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Sets the I2C device slave address
 *
 * \param [IN]: addr
 */
void sx9500SetDeviceAddr( uint8_t addr );

/*!
 * \brief Gets the I2C device slave address
 *
 * \retval: addr Current device slave address
 */
uint8_t sx9500GetDeviceAddr( void );

/*!
 * \Goes into a loop until a successful capacitive proximity detection
 *
 */
void sx9500LockUntilDetection( void );

#endif  // __SX1509_H__

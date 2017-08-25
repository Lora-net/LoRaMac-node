/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Implements the generic SPI driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __SPI_H__
#define __SPI_H__

#include "nrf_drv_spi.h"

/*!
 * SPI driver structure definition
 */
struct Spi_s {
	nrf_drv_spi_t			Instance;
	nrf_drv_spi_config_t	Config;
	bool					initialized;
};

/*!
 * SPI object type definition
 */
typedef struct Spi_handle Spi_t;

/*!
 * \brief Initializes the SPI object and MCU peripheral
 *
 * \remark When NSS pin is software controlled set the pin name to NC otherwise
 *         set the pin name to be used.
 *
 * \param [IN] obj  SPI object
 * \param [IN] mosi SPI MOSI pin name to be used
 * \param [IN] miso SPI MISO pin name to be used
 * \param [IN] sclk SPI SCLK pin name to be used
 * \param [IN] nss  SPI NSS pin name to be used
 */
void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss );

/*!
 * \brief De-initializes the SPI object and MCU peripheral
 *
 * \param [IN] obj SPI object
 */
void SpiDeInit( Spi_t *obj );

/*!
 * \brief Writes outData buffer to register addr
 *
 * \param [IN] obj     SPI object
 * \param [IN] addr    Register address
 * \param [IN] outData Bytes to be sent
 * \param [IN] outLen  Number of bytes to be sent
 */
void SpiOut( Spi_t *obj, uint8_t addr, uint8_t *outData, uint8_t outLen );

/*!
 * \brief Read inData buffer from register addr
 *
 * \param [IN] obj     SPI object
 * \param [IN] addr    Register address
 * \param [OUT] inData Bytes read
 * \param [IN] inLen   Maximum number of bytes to be read (size of buffer)
 */
void SpiIn( Spi_t *obj, uint8_t addr, uint8_t *inData, uint8_t inLen);

#endif  // __SPI_H__

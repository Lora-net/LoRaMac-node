/*!
 * \file      nvmm.h
 *
 * \brief     None volatile memory management module
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
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 */
#ifndef __NVMM_H__
#define __NVMM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*!
 * \brief Writes data to given data block.
 *
 * \param[IN] src    Pointer to the source of data to be copied.
 * \param[IN] size   Number of bytes to copy.
 * \param[IN] offset Relative NVM offset.
 *
 * \retval           Status of the operation
 */
uint16_t NvmmWrite( uint8_t* src, uint16_t size, uint16_t offset );

/*!
 * \brief Reads from data block to destination pointer.
 *
 * \param[IN] dst    Pointer to the destination array where the content is to be copied.
 * \param[IN] size   Number of bytes to copy.
 * \param[IN] offset Relative NVM offset.
 *
 * \retval           Status of the operation
 */
uint16_t NvmmRead( uint8_t* dest, uint16_t size, uint16_t offset );

/*!
 * \brief Verfies the CRC 32 of a data block. The function assumes that the
 *        crc32 is at the end of the block with 4 bytes.
 *
 * \param[IN] size   Length of the block.
 * \param[IN] offset Address offset of the NVM.
 *
 * \retval           Status of the operation
 */
bool NvmmCrc32Check( uint16_t size, uint16_t offset );

/*!
 * \brief Invalidates the CRC 32 of a data block. The function assumes that the
 *        crc32 is at the end of the block with 4 bytes.
 *
 * \param[IN] size   Length of the block.
 * \param[IN] offset Address offset of the NVM.
 *
 * \retval           Status of the operation
 */
bool NvmmReset( uint16_t size, uint16_t offset );

#ifdef __cplusplus
}
#endif

#endif // __NVMM_H__

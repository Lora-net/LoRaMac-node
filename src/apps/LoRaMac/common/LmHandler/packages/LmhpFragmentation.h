/*!
 * \file      LmhpFragmentation.h
 *
 * \brief     Implements the LoRa-Alliance fragmented data block transport package
 *            Specification: https://lora-alliance.org/sites/default/files/2018-09/fragmented_data_block_transport_v1.0.0.pdf
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
 *              (C)2013-2018 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 */
#ifndef __LMHP_FRAGMENTATION_H__
#define __LMHP_FRAGMENTATION_H__

#include "LoRaMac.h"
#include "LmHandlerTypes.h"
#include "LmhPackage.h"

/*!
 * Fragmentation data block transport package identifier.
 *
 * \remark This value must be unique amongst the packages
 */
#define PACKAGE_ID_FRAGMENTATION                    3

/*!
 * Fragmentation package parameters
 */
typedef struct LmhpFragmentationParams_s
{
    /*!
     * Pointer to the un-fragmented received buffer.
     */
    uint8_t *Buffer;
    /*!
     * Size of the un-fragmented received buffer.
     */
    uint32_t BufferSize;
    /*!
     *
     */
    void ( *OnProgress )( uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost );
    /*!
     *
     */
    void ( *OnDone )( int32_t status, uint8_t *file, uint32_t size );
}LmhpFragmentationParams_t;

LmhPackage_t *LmhpFragmentationPackageFactory( void );

#endif // __LMHP_FRAGMENTATION_H__

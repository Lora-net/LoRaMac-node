/*!
 * \file      FragDecoder.h
 *
 * \brief     Implements the LoRa-Alliance fragmentation decoder
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
 * \author    Fabien Holin ( Semtech )
 * \author    Miguel Luis ( Semtech )
 */
#ifndef __FRAG_DECODER_H__
#define __FRAG_DECODER_H__

#include <stdint.h>

/*!
 * Maximum number of fragment that can be handled.
 *
 * \remark This parameter has an impact on the memory footprint.
 */
#define FRAG_MAX_NB                                 20

/*!
 * Maximum fragment size that can be handled.
 *
 * \remark This parameter has an impact on the memory footprint.
 */
#define FRAG_MAX_SIZE                               50

/*!
 * Maximum number of extra frames that can be handled.
 *
 * \remark This parameter has an impact on the memory footprint.
 */
#define FRAG_MAX_REDUNDANCY                         5

#define FRAG_SESSION_FINISHED                       ( int32_t )0
#define FRAG_SESSION_NOT_STARTED                    ( int32_t )-2
#define FRAG_SESSION_ONGOING                        ( int32_t )-1

typedef struct sFragDecoderStatus
{
    uint16_t FragNbRx;
    uint16_t FragNbLost;
    uint16_t FragNbLastRx;
    uint8_t MatrixError;
}FragDecoderStatus_t;

/*!
 * \brief Initializes the fragmentation decoder
 *
 * \param [IN] fragNb     Number of expected fragments (without redundancy packets)
 * \param [IN] fragSize   Size of a fragment
 * \param [IN] redundancy Maximum number of redundancy packets
 * \param [IN] file       Pointer to the final un-coded buffer
 * \param [IN] fileSize   Pointer to the final un-coded buffer
 */
void FragDecoderInit( uint16_t fragNb, uint8_t fragSize, uint16_t redundancy, uint8_t *file, uint32_t fileSize );

/*!
 * \brief Function to decode and reconstruct the binary file
 *        Called for each receive frame
 * 
 * \param [IN] fragCounter Fragment counter
 * \param [IN] rawData     Pointer to the fragment to be processed (length = FRAG_MAX_SIZE)
 *
 * \retval fragCounter Fragment counter
 */
int32_t FragDecoderProcess( uint16_t fragCounter, uint8_t *rawData );

/*!
 * \brief Gets the current fragmentation status
 * 
 * \retval status Fragmentation decoder status
 */
FragDecoderStatus_t FragDecoderGetStatus( void );

#endif // __FRAG_DECODER_H__
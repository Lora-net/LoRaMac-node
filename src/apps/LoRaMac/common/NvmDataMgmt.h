/*!
 * \file      NvmDataMgmt.h
 *
 * \brief     NVM context management implementation
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
 *
 * \defgroup  NVMDATAMGMT NVM context management implementation
 *            This module implements the NVM context handling
 * \{
 */
#ifndef __NVMDATAMGMT_H__
#define __NVMDATAMGMT_H__

/*!
 * \brief NVM Management event.
 *
 * \param [IN] notifyFlags Bitmap which contains the information about modules that
 *                         changed.
 */
void NvmDataMgmtEvent( uint16_t notifyFlags );

/*!
 * \brief Function which stores the MAC data into NVM, if required.
 *
 * \retval Number of bytes which were stored.
 */
uint16_t NvmDataMgmtStore( void );

/*!
 * \brief Function which restores the MAC data from NVM, if required.
 *
 * \retval Number of bytes which were restored.
 */
uint16_t NvmDataMgmtRestore(void );

/*!
 * \brief Resets the NVM data.
 *
 * \retval Returns true, if successful.
 */
bool NvmDataMgmtFactoryReset( void );

/* \} */

#endif // __NVMDATAMGMT_H__

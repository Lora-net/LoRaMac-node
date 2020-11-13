/*!
 * \file      LmhpCompliance.h
 *
 * \brief     Implements the LoRa-Alliance certification protocol handling
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
#ifndef __LMHP_COMPLIANCE__
#define __LMHP_COMPLIANCE__

#include "LoRaMac.h"
#include "LmHandlerTypes.h"
#include "LmhPackage.h"

/*!
 * Compliance package identifier.
 * 
 * \remark This value must be unique amongst the packages
 */
#define PACKAGE_ID_COMPLIANCE                       0

/*!
 * Compliance test protocol handler parameters
 */
typedef struct LmhpComplianceParams_s
{
    /*!
     * Current firmware version
     */
    Version_t FwVersion;
    /*!
     *
     */
    void ( *OnTxPeriodicityChanged )( uint32_t periodicity );
    /*!
     *
     */
    void ( *OnTxFrameCtrlChanged )( LmHandlerMsgTypes_t isTxConfirmed );
    /*!
     *
     */
    void ( *OnPingSlotPeriodicityChanged )( uint8_t pingSlotPeriodicity );
}LmhpComplianceParams_t;

LmhPackage_t *LmphCompliancePackageFactory( void );

#endif // __LMHP_COMPLIANCE__

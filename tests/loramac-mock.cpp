/*!
 * \file      LoRaMac.c
 *
 * \brief     LoRa MAC layer implementation
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
 * Modified by Medad Newman for mock support
 */

#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "utilities.h"
#include "region/Region.h"
#include "LoRaMacClassB.h"
#include "LoRaMacCrypto.h"
#include "secure-element.h"
#include "LoRaMacTest.h"
#include "LoRaMacTypes.h"
#include "LoRaMacConfirmQueue.h"
#include "LoRaMacHeaderTypes.h"
#include "LoRaMacMessageTypes.h"
#include "LoRaMacParser.h"
#include "LoRaMacCommands.h"
#include "LoRaMacAdr.h"
#include "LoRaMacSerializer.h"
#include "radio.h"

#include "LoRaMac.h"
}

LoRaMacStatus_t LoRaMacStart(void)
{
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacStop(void)
{
    return LORAMAC_STATUS_OK;
}

LoRaMacNvmData_t get_LoRaMacNvmData()
{
    auto returnValue = mock().actualCall(__func__).returnPointerValue();
    return *(LoRaMacNvmData_t *)(returnValue);
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm(MibRequestConfirm_t *mibGet)
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    LoRaMacNvmData_t data = get_LoRaMacNvmData();

    mibGet->Param.Contexts = &data;

    return status;
}

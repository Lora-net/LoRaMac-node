/*!
 * \file      NvmDataMgmt.c
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
 */

#include "nvmm.h"
#include "NvmDataMgmt.h"
#include "LoRaWAN_config_switcher.h"
#include "secure-element.h"

/*!
 * Enables/Disables the context storage management storage.
 * Must be enabled for LoRaWAN 1.0.4 or later.
 */
bool context_management_enabled = true;

#define PRINT_TIME()                                                                 \
    do                                                                               \
    {                                                                                \
        SysTime_t time = SysTimeGet();                                               \
        printf("%ld.%03d: ", time.Seconds - UNIX_GPS_EPOCH_OFFSET, time.SubSeconds); \
    } while (0)

#define PRINT(...)            \
    do                        \
    {                         \
        PRINT_TIME();         \
        printf("[DBG    ] "); \
        printf(__VA_ARGS__);  \
    } while (0)

static uint16_t NvmNotifyFlags = 0;

void NvmDataMgmtEvent(uint16_t notifyFlags)
{
    NvmNotifyFlags = notifyFlags;
}

uint16_t NvmDataMgmtStore(void)
{
    if (context_management_enabled == true)
    {

        // Input checks
        if (NvmNotifyFlags == LORAMAC_NVM_NOTIFY_FLAG_NONE)
        {
            // There was no update.
            return 0;
        }

        if (LoRaMacStop() != LORAMAC_STATUS_OK)
        {
            return 0;
        }

        // Store frame count here

        MibRequestConfirm_t mibReq;
        mibReq.Type = MIB_NVM_CTXS;
        LoRaMacMibGetRequestConfirm(&mibReq);
        LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

        uint16_t bytes_written;

        // read nvm fcount for the registered device
        network_keys_t current_keys = get_current_network_keys(); // TODO: this should not be done here. It should read the current mac state
        registered_devices_t registered_device = get_current_network();

        /* update current mac state to be saved */
        current_keys.frame_count = nvm->Crypto.FCntList.FCntUp;
        current_keys.ReceiveDelay1 = nvm->MacGroup2.MacParams.ReceiveDelay1;
        current_keys.ReceiveDelay2 = nvm->MacGroup2.MacParams.ReceiveDelay2;

        // now update CRC before writing to EEPROM
        current_keys.Crc32 = Crc32((uint8_t *)&current_keys, sizeof(current_keys) - sizeof(current_keys.Crc32));

        // Now write current keys for this network(including frame count) to EEPROM into the right place in the EEPROM
        bytes_written = NvmmUpdate((uint8_t *)&current_keys, sizeof(network_keys_t), registered_device * sizeof(network_keys_t));

        // Reset notification flags
        NvmNotifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;

        // Resume LoRaMac
        LoRaMacStart();

        return bytes_written;
    }
    else
    {
        return 0;
    }
}

uint16_t NvmDataMgmtRestore(void)
{
    if (context_management_enabled == true)
    {
        MibRequestConfirm_t mibReq;
        mibReq.Type = MIB_NVM_CTXS;
        LoRaMacMibGetRequestConfirm(&mibReq);
        LoRaMacNvmData_t *nvm = mibReq.Param.Contexts;

        // Read from eeprom the region/networks fcount
        uint16_t bytes_read;

        // read nvm fcount for the registered device
        registered_devices_t registered_device = get_current_network();
        network_keys_t current_keys;
        bytes_read = NvmmRead((uint8_t *)&current_keys, sizeof(network_keys_t), registered_device * sizeof(network_keys_t));

        // check if NVM crc matches for fcount
        // if crc fails, frame count = 0
        // else, use the EEPROM stored frame count
        if (is_crc_correct(sizeof(current_keys), &current_keys) == false)
        {
            current_keys = get_current_network_keys();
        }

        /**
         * Increment the Fcount in EEPROM and locally to be absolutely sure that every
         * transmission increments the fcount. There could be instances when tx is done,
         * but incrementation of fcount in EEPROM does not happen due to brownout.
         * So to be absolutely sure, increment fcount in EEPROM during the initialisation 
         * phase that always happens after boot.
         * 
         */
        current_keys.frame_count += 1;
        // now update CRC before writing to EEPROM
        current_keys.Crc32 = Crc32((uint8_t *)&current_keys, sizeof(current_keys) - sizeof(current_keys.Crc32));
        // Now write current keys for this network(including frame count) to EEPROM into the right place in the EEPROM
        NvmmUpdate((uint8_t *)&current_keys, sizeof(network_keys_t), registered_device * sizeof(network_keys_t));

        /* Now update the mac state. We set the frame count, dev_addr, rx1 delay and rx2 delay
         * and  FNwkSIntKey,SNwkSIntKey, NwkSEncKey, AppSKey. The rest we leave as default for
         * this region.
         */
        nvm->Crypto.FCntList.FCntUp = current_keys.frame_count;
        nvm->MacGroup2.DevAddr = current_keys.DevAddr;
        nvm->MacGroup2.MacParams.ReceiveDelay1 = current_keys.ReceiveDelay1;
        nvm->MacGroup2.MacParams.ReceiveDelay2 = current_keys.ReceiveDelay2;
        SecureElementSetKey(F_NWK_S_INT_KEY, current_keys.FNwkSIntKey_SNwkSIntKey_NwkSEncKey);
        SecureElementSetKey(S_NWK_S_INT_KEY, current_keys.FNwkSIntKey_SNwkSIntKey_NwkSEncKey);
        SecureElementSetKey(NWK_S_ENC_KEY, current_keys.FNwkSIntKey_SNwkSIntKey_NwkSEncKey);
        SecureElementSetKey(APP_S_KEY, current_keys.AppSKey);

        return bytes_read;
    }
    return 0;
}

/**
 * @file compression_structs.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "LoRaMac.h"


typedef struct
{
    uint16_t compressed_data_size;
    char compressed_data[700];
    LoRaMacCryptoNvmData_t Crypto;
    LoRaMacNvmDataGroup1_t MacGroup1;
    RegionNvmDataGroup1_t RegionGroup1;
    uint32_t Crc32; // CRC32 value of the nvm_data_t data structure.
} nvm_data_t;

typedef struct
{
    LoRaMacNvmDataGroup2_t MacGroup2;
    SecureElementNvmData_t SecureElement;
    RegionNvmDataGroup2_t RegionGroup2;
    LoRaMacClassBNvmData_t ClassB;

} rarely_changing_t;
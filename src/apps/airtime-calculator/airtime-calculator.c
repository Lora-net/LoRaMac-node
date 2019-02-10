#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sx1276/sx1276.h"
#include "region/RegionEU868.h"
#include "LoRaMacMessageTypes.h"

#define DR_MIN 0 // SF 12
#define DR_MAX 6 // SF 7

extern SX1276_t SX1276;

uint32_t getAirTime(uint8_t datarate, uint8_t payloadlen);

uint32_t GetBandwidth( uint32_t drIndex )
{
    switch( BandwidthsEU868[drIndex] )
    {
        default:
        case 125000:
            return 0;
        case 250000:
            return 1;
        case 500000:
            return 2;
    }
}

const char *datarate_desc(uint8_t dr)
{
    char *datarates[7] = {
        "SF12",
        "SF11",
        "SF10",
        "SF9",
        "SF8",
        "SF7/BW125",
        "SF7/BW250",
    };
    return datarates[dr];
}

void usage(char**argv)
{
    fprintf(stderr,"Usage: %s [payloadsize [datarate]]\n",argv[0]);
    exit(0);
}

int main(int argc, char **argv)
{
    if ( argc > 1 && !strcmp(argv[1],"-h") ) usage(argv);

    uint8_t payloadlen_min = 52, payloadlen_max = 52;
    uint8_t dr_min = DR_MIN, dr_max = DR_MAX;

    if ( argc == 1 )
    {
        payloadlen_min = 0;
        payloadlen_max = MaxPayloadOfDatarateEU868[DR_MAX];
    } else if ( argc == 2 ) {
        payloadlen_max = payloadlen_min = atoi(argv[1]);
    } else {
        payloadlen_max = payloadlen_min = atoi(argv[1]);
        dr_min = dr_max = atoi(argv[2]);
        dr_min = MAX(dr_min,DR_MIN);
        dr_max = MIN(dr_max,DR_MAX);
    }

    printf("payloadlen,datarate,datarate_desc,airtime\n");
    for ( uint8_t datarate = DR_MIN;datarate <= DR_MAX;datarate++ )
    {
        uint8_t dr_max_len = MaxPayloadOfDatarateEU868[datarate];
        for ( uint8_t payloadlen = payloadlen_min;payloadlen<=payloadlen_max && payloadlen<=dr_max_len;payloadlen++ )
        {
            uint32_t airtime = getAirTime(datarate,payloadlen);
            printf("%d,%d,%s,%d\n",payloadlen,datarate,datarate_desc(datarate),airtime);
        }
    }
}

uint32_t getAirTime(uint8_t datarate, uint8_t payloadlen)
{
    RadioModems_t modem = MODEM_LORA;

    size_t headersize = LORAMAC_MHDR_FIELD_SIZE + LORAMAC_FHDR_DEV_ADD_FIELD_SIZE + LORAMAC_FHDR_F_CTRL_FIELD_SIZE + LORAMAC_FHDR_F_CNT_FIELD_SIZE;
    if ( payloadlen > 0 ) headersize += LORAMAC_F_PORT_FIELD_SIZE;
    headersize += 4; // mic len

    uint8_t pktLen = payloadlen + headersize;

    int8_t phyDr = DataratesEU868[datarate];
    uint32_t bandwidth = GetBandwidth(datarate);

    if( bandwidth > 2 )
    {
        printf("Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported\n");
        return ~0;
    }
    bandwidth += 7;

    SX1276.Settings.LoRa.Bandwidth = bandwidth;
    SX1276.Settings.LoRa.Datarate = phyDr;
    SX1276.Settings.LoRa.PreambleLen = 8;
    SX1276.Settings.LoRa.CrcOn = true;
    SX1276.Settings.LoRa.FixLen = false;
    SX1276.Settings.LoRa.Coderate = 1;
    SX1276.Settings.LoRa.TxTimeout = 6000;

    if( phyDr > 12 )
    {
        phyDr = 12;
    }
    else if( phyDr < 6 )
    {
        phyDr = 6;
    }
    if( ( ( bandwidth == 7 ) && ( ( phyDr == 11 ) || ( phyDr == 12 ) ) ) ||
        ( ( bandwidth == 8 ) && ( phyDr == 12 ) ) )
    {
        SX1276.Settings.LoRa.LowDatarateOptimize = 0x01;
    }
    else
    {
        SX1276.Settings.LoRa.LowDatarateOptimize = 0x00;
    }

    uint32_t airtime = SX1276GetTimeOnAir( modem, pktLen );
    return airtime;
}


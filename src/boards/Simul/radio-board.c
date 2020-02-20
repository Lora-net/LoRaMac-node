
#include <stddef.h>
#include "radio.h"
/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    NULL, //SX1276Init,
    NULL, //SX1276GetStatus,
    NULL, //SX1276SetModem,
    NULL, //SX1276SetChannel,
    NULL, //SX1276IsChannelFree,
    NULL, //SX1276Random,
    NULL, //SX1276SetRxConfig,
    NULL, //SX1276SetTxConfig,
    NULL, //SX1276CheckRfFrequency,
    NULL, //SX1276GetTimeOnAir,
    NULL, //SX1276Send,
    NULL, //SX1276SetSleep,
    NULL, //SX1276SetStby,
    NULL, //SX1276SetRx,
    NULL, //SX1276StartCad,
    NULL, //SX1276SetTxContinuousWave,
    NULL, //SX1276ReadRssi,
    NULL, //SX1276Write,
    NULL, //SX1276Read,
    NULL, //SX1276WriteBuffer,
    NULL, //SX1276ReadBuffer,
    NULL, //SX1276SetMaxPayloadLength,
    NULL, //SX1276SetPublicNetwork,
    NULL, //SX1276GetWakeupTime,
    NULL, // void ( *IrqProcess )( void )
    NULL, // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
    NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};

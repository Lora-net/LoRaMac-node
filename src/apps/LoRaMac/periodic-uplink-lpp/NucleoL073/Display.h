#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "LmHandler.h"

void DisplayNetworkParametersUpdate( CommissioningParams_t* commissioningParams );

void DisplayMacMcpsRequestUpdate( LoRaMacStatus_t status, McpsReq_t *mcpsReq );

void DisplayMacMlmeRequestUpdate( LoRaMacStatus_t status, MlmeReq_t *mlmeReq );

void DisplayJoinRequestUpdate( LmHandlerJoinParams_t* params );

void DisplayTxUpdate( LmHandlerTxParams_t* params );

void DisplayRxUpdate( LmHandlerAppData_t* appData, LmHandlerRxParams_t* params );

void DisplayBeaconUpdate( LoRaMAcHandlerBeaconParams_t* params );

void DisplayClassUpdate( DeviceClass_t deviceClass );

void DisplayVersion( void );

#endif // __DISPLAY_H__

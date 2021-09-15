/**
 * @file callbacks.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "LmHandler.h"
#include "LmhpCompliance.h"
#include "LmHandlerMsgDisplay.h"


void OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size);
void OnNetworkParametersChange(CommissioningParams_t *params);
void OnMacMcpsRequest(LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn);
void OnMacMlmeRequest(LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn);
void OnJoinRequest(LmHandlerJoinParams_t *params);
void OnTxData(LmHandlerTxParams_t *params);
void OnClassChange(DeviceClass_t deviceClass);
void OnBeaconStatusChange(LoRaMAcHandlerBeaconParams_t *params);
#if (LMH_SYS_TIME_UPDATE_NEW_API == 1)
void OnSysTimeUpdate(bool isSynchronized, int32_t timeCorrection);
#else
void OnSysTimeUpdate(void);
#endif
/******************************************************************************
*  Copyright 2017 - 2018, Opulinks Technology Ltd.
*  ----------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2018
******************************************************************************/

#ifndef _BLEWIFI_SERVER_APP_GATT_H_
#define _BLEWIFI_SERVER_APP_GATT_H_

#include "ble_msg.h"


#define BLEWIFI_INFO(fmt, ...)      tracer_log(LOG_LOW_LEVEL, fmt, ##__VA_ARGS__)
#define BLEWIFI_WARN(fmt, ...)      tracer_log(LOG_MED_LEVEL, fmt, ##__VA_ARGS__)
#define BLEWIFI_ERROR(fmt, ...)     tracer_log(LOG_HIGH_LEVEL, fmt, ##__VA_ARGS__)


// BLE part
/*
BLE Service UUID
*/
#define BLEWIFI_BLE_UUID_SERVICE        0xFFF0
#define BLEWIFI_BLE_UUID_DATA_IN        0xFFF2
#define BLEWIFI_BLE_UUID_DATA_OUT       0xFFF1

/* Device Name
The max length of device name is 29 bytes.
    method 1: use prefix + mac address
        Ex: OPL_33:44:55:66

    method 2: full name
        Ex: OPL1000
*/
#define BLEWIFI_BLE_DEVICE_NAME_METHOD          2           // 1 or 2
#define BLEWIFI_BLE_DEVICE_NAME_POSTFIX_COUNT   4           // for method 1 "OPL_33:44:55:66"
#define BLEWIFI_BLE_DEVICE_NAME_PREFIX          "SENSSUN X"      // for method 1 "OPL_33:44:55:66"
#define BLEWIFI_BLE_DEVICE_NAME_FULL            "SENSSUN X"   // for method 2

/* Advertisement Interval in working mode:
1000 (ms) / 0.625 (ms) = 1600 = 0x640
0xFFFF is deifined 30 min in dirver part
*/
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_MIN  0x640  // For 1 sec
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_MAX  0x640  // For 1 sec

/* For the power saving of Advertisement Interval
0xFFFF is deifined 30 min in dirver part
*/
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_PS_MIN   0xFFFF  // 30 min
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_PS_MAX   0xFFFF  // 30 min

/* For the initial settings of Advertisement Interval */
#ifdef __BLEWIFI_TRANSPARENT__
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_INIT_MIN     BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_PS_MIN
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_INIT_MAX     BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_PS_MAX
#else
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_INIT_MIN     BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_MIN
#define BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_INIT_MAX     BLEWIFI_BLE_ADVERTISEMENT_INTERVAL_MAX
#endif

enum
{
    GATT_IDX_SVC,
        
	GATT_IDX_SERVICE_CHANGE_CHAR,
	GATT_IDX_SERVICE_CHANGE_VAL,
	GATT_IDX_SERVICE_CHANGE_CFG,

    GATT_IDX_TOP
};

enum
{
    GAP_IDX_SVC,

	GAP_IDX_DEVICE_NAME_CHAR,
	GAP_IDX_DEVICE_NAME_VAL,

	GAP_IDX_APPEARANCE_CHAR,
	GAP_IDX_APPEARANCE_VAL,

	GAP_IDX_CONN_PARAM_CHAR,
	GAP_IDX_CONN_PARAM_VAL,

    GAP_IDX_TOP
};

enum
{
    BWP_IDX_SVC,

	BWP_IDX_DATA_IN_CHAR,
	BWP_IDX_DATA_IN_VAL,

	BWP_IDX_DATA_OUT_CHAR,
	BWP_IDX_DATA_OUT_VAL,
	BWP_IDX_DATA_OUT_CFG,
	
    BWP_IDX_TOP
};

// BLE GAP Device Name (default)
#define BLEWIFI_BLE_GAP_DEVICE_NAME     "SENSSUN X"

typedef void (*T_BleWifi_Ble_MsgHandler_Fp)(TASK task, MESSAGEID id, MESSAGE message);
typedef struct
{
    uint32_t ulEventId;
    T_BleWifi_Ble_MsgHandler_Fp fpFunc;
} T_BleWifi_Ble_MsgHandlerTbl;

void BleWifi_Ble_ServerAppGattMsgHandler(TASK task, MESSAGEID id, MESSAGE message);

void BleWifi_Ble_GattIndicateServiceChange(UINT16 conn_hdl);

void BleWifi_Ble_AppUpdateDevName(UINT8 *name, UINT16 len);

#endif

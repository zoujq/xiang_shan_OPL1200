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

#ifndef _BLEWIFI_SERVER_APP_H_
#define _BLEWIFI_SERVER_APP_H_

#include "ble.h"
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


// Minimum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     	100
// Maximum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     	200
// Maximum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY			0
// Maximum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SUPERVERSION_TIMEOUT   	500

#define LE_GATT_DATA_OUT_BUF_SIZE				1024

#define BLE_ADV_SCAN_BUF_SIZE                   31


// the BLE state is used in the Le stack task
enum
{
	APP_STATE_INIT = 0,
	APP_STATE_IDLE,
	APP_STATE_ADVERTISING,
	APP_STATE_ADVERTISING_TIME_CHANGE,
	APP_STATE_CONNECTED,

    APP_STATE_TOP
};

// the BLE message is used in the Le stack task
enum
{
    BLEWIFI_APP_MSG_BASE = 0x4000,

    BLEWIFI_APP_MSG_INITIALIZING,
    BLEWIFI_APP_MSG_ENTER_ADVERTISING,
    BLEWIFI_APP_MSG_EXIT_ADVERTISING,
    BLEWIFI_APP_MSG_DISCONNECTION,
    BLEWIFI_APP_MSG_CHANGE_ADVERTISING_TIME,

    BLEWIFI_APP_MSG_SEND_DATA,          // copy data to buffer
    BLEWIFI_APP_MSG_SEND_TO_PEER,       // send data from buffer to peer
    BLEWIFI_APP_MSG_SEND_TO_PEER_CFM,   // send data is confirmed

    BLEWIFI_APP_MSG_TOP
};


typedef struct
{
	UINT16		len;
	UINT8		*data;
} BLEWIFI_MESSAGE_T;

typedef struct
{
	UINT32			test_num;
	UINT16			ridx;
	UINT16			pidx;
	UINT16			send_hdl;
    UINT8			sending;
    UINT8			*send_buf;
	UINT8			buf[LE_GATT_DATA_OUT_BUF_SIZE];
} BLEWIFI_DATA_OUT_STORE_T;

typedef struct
{
	UINT16			len;
	UINT8			buf[BLE_ADV_SCAN_BUF_SIZE];
} BLE_ADV_SCAN_T;

typedef struct
{
	UINT16			interval_min;
	UINT16			interval_max;
} BLE_ADV_TIME_T;

typedef struct
{
	TASKPACK		task;
	UINT16			state;
	UINT16			conn_hdl;
	LE_BT_ADDR_T	bt_addr;

	UINT16			curr_mtu;
	BOOL			encrypted;
    BOOL			paired;

    UINT16			min_itvl;
    UINT16			max_itvl;
    UINT16			latency;
    UINT16			sv_tmo;

	BLEWIFI_DATA_OUT_STORE_T store;
    
	BLE_ADV_SCAN_T	adv_data;
    BLE_ADV_SCAN_T	scn_data;
    
} BLE_APP_DATA_T;


BLE_APP_DATA_T* BleWifi_Ble_GetEntity(void);
UINT16 BleWifi_Ble_GetBufFreeSize(void);

void BleWifi_Ble_SendAppMsgToBle(UINT32 id, UINT16 len, void *data);
void BleWifi_Ble_ServerAppInit(void);

#endif

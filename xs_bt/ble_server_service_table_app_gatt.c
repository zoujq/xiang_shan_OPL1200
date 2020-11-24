/******************************************************************************
*  Copyright 2017 - 2018, Opulinks Technology Ltd.
*  ---------------------------------------------------------------------------
*  Statement:
*  ----------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Opulinks Technology Ltd. (C) 2018
******************************************************************************/

#include "ble_server_service_table_app.h"
#include "ble_server_service_table_app_gatt.h"
#include "ble_gatt_if.h"
#include "ble_uuid.h"


#define BLE_DEVICE_NAME                    "SENSSUN01"


// This is used for GATT service
static UINT16 gGattSvcUuid = ATT_SVC_GENERIC_ATTRIBUTE;

static UINT16 gGattServiceChangeUuid      = ATT_CHAR_SERVICE_CHANGED;
static UINT8  gGattServiceChangeCharVal[] = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_IND, ATT_CHAR_SERVICE_CHANGED);
static UINT8  gGattServiceChangeVal[4]    = {0, 0, 0, 0};
static UINT16 gGattServiceChangeClientCfg = 0;

// This is used for GAP service 
static UINT16 gGapSvcUuid = ATT_SVC_GENERIC_ACCESS;

static UINT16 gGapDeviceNameUuid          = ATT_CHAR_DEVICE_NAME;
static UINT8  gGapDeviceNameCharVal[]     = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD | LE_GATT_CHAR_PROP_WR, ATT_CHAR_DEVICE_NAME);
static UINT8  gGapDeviceNameVal[31]       = BLE_DEVICE_NAME;

static UINT16 gGapAppearanceUuid          = ATT_CHAR_APPEARANCE;
static UINT8  gGapAppearanceCharVal[]     = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD, ATT_CHAR_APPEARANCE);
static UINT8  gGapAppearanceVal[2]        = {0, 0};

static UINT16 gGapConnParamUuid           = ATT_CHAR_PERIPH_PREF_CON_PARAM;
static UINT8  gGapConnParamCharVal[]      = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_RD, ATT_CHAR_PERIPH_PREF_CON_PARAM);
static UINT16 gGapConnParamVal[4]         = {DEFAULT_DESIRED_MIN_CONN_INTERVAL, DEFAULT_DESIRED_MAX_CONN_INTERVAL, DEFAULT_DESIRED_SLAVE_LATENCY, DEFAULT_DESIRED_SUPERVERSION_TIMEOUT};

// // This is used for XS Pressure service
#define XS_UART_SERVICE_UUID 0xFFF0
#define XS_UART_NOTIFY_CHARA_UUID 0xFFF1
#define XS_UART_WIRTE_CHARA_UUID 0xFFF2
#define BLPS_BP_MEAS_MAX_LEN            (19)

static UINT16 gBleWifiSvcUuid             = XS_UART_SERVICE_UUID;

static UINT16 gBleWifiDataInUuid          = XS_UART_WIRTE_CHARA_UUID;
static UINT8  gBleWifiDataInCharVal[]     = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_WR_NO_RESP | LE_GATT_CHAR_PROP_WR, XS_UART_WIRTE_CHARA_UUID);
static UINT8  gBleWifiDataInVal[LE_ATT_MAX_MTU];

static UINT16 gBleWifiDataOutUuid         = XS_UART_NOTIFY_CHARA_UUID;
static UINT8  gBleWifiDataOutCharVal[]    = CHAR_DECL_UUID16_ATTR_VAL(LE_GATT_CHAR_PROP_NTF, XS_UART_NOTIFY_CHARA_UUID);
static UINT8  gBleWifiDataOutVal[LE_ATT_MAX_MTU];
static UINT16 gBleWifiDataOutClientCfg    = 1;



static LE_GATT_ATTR_T gGattSvcDb[GATT_IDX_TOP] =
{
    // GATT Service Declaration
    [GATT_IDX_SVC]                         = PRIMARY_SERVICE_DECL_UUID16(&gGattSvcUuid),
    // GATT Service Change Characteristic 
    [GATT_IDX_SERVICE_CHANGE_CHAR]         = CHARACTERISTIC_DECL_UUID16(gGattServiceChangeCharVal),
    [GATT_IDX_SERVICE_CHANGE_VAL]          = CHARACTERISTIC_UUID16(&gGattServiceChangeUuid, 0, 0, sizeof(gGattServiceChangeVal), gGattServiceChangeVal),
    [GATT_IDX_SERVICE_CHANGE_CFG]          = CHAR_CLIENT_CONFIG_DESCRIPTOR(LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, &gGattServiceChangeClientCfg)
};

static LE_GATT_ATTR_T gGapSvcDb[GAP_IDX_TOP] =
{
    // GAP Service Declaration
    [GAP_IDX_SVC]                          = PRIMARY_SERVICE_DECL_UUID16(&gGapSvcUuid),
    // GAP Device Name Characteristic
    [GAP_IDX_DEVICE_NAME_CHAR]             = CHARACTERISTIC_DECL_UUID16(gGapDeviceNameCharVal),
    [GAP_IDX_DEVICE_NAME_VAL]              = CHARACTERISTIC_UUID16(&gGapDeviceNameUuid, LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, sizeof(gGapDeviceNameVal), sizeof(BLE_DEVICE_NAME) - 1, gGapDeviceNameVal),
    // GAP Appearance Characteristic
    [GAP_IDX_APPEARANCE_CHAR]              = CHARACTERISTIC_DECL_UUID16(gGapAppearanceCharVal),
    [GAP_IDX_APPEARANCE_VAL]               = CHARACTERISTIC_UUID16(&gGapAppearanceUuid, LE_GATT_PERMIT_READ, 0, 2, gGapAppearanceVal),
    // GAP Connection Parameter Characteristic Declaration
    [GAP_IDX_CONN_PARAM_CHAR]              = CHARACTERISTIC_DECL_UUID16(gGapConnParamCharVal),
    [GAP_IDX_CONN_PARAM_VAL]               = CHARACTERISTIC_UUID16(&gGapConnParamUuid, LE_GATT_PERMIT_READ, 0, 8, gGapConnParamVal)
};

static LE_GATT_ATTR_T gBwpSvcDb[BWP_IDX_TOP] =
{
    // BLEWIFI Service Declaration
    [BWP_IDX_SVC]                  = PRIMARY_SERVICE_DECL_UUID16(&gBleWifiSvcUuid),
    // BLEWIFI Data In Characteristic
    [BWP_IDX_DATA_IN_CHAR]         = CHARACTERISTIC_DECL_UUID16(gBleWifiDataInCharVal),
    [BWP_IDX_DATA_IN_VAL]          = CHARACTERISTIC_UUID16(&gBleWifiDataInUuid, LE_GATT_PERMIT_AUTHOR_WRITE, sizeof(gBleWifiDataInVal), 0, gBleWifiDataInVal),
    // BLEWIFI Data Out Characteristic
    [BWP_IDX_DATA_OUT_CHAR]        = CHARACTERISTIC_DECL_UUID16(gBleWifiDataOutCharVal),
    [BWP_IDX_DATA_OUT_VAL]         = CHARACTERISTIC_UUID16(&gBleWifiDataOutUuid, 0, sizeof(gBleWifiDataOutVal), 0, gBleWifiDataOutVal),
    [BWP_IDX_DATA_OUT_CFG]         = CHAR_CLIENT_CONFIG_DESCRIPTOR(LE_GATT_PERMIT_AUTHOR_READ | LE_GATT_PERMIT_AUTHOR_WRITE, &gBleWifiDataOutClientCfg)
};



static LE_GATT_SERVICE_T *gGattSvc = 0;
static LE_GATT_SERVICE_T *gGapSvc = 0;
static LE_GATT_SERVICE_T *gBlpsSvc = 0;


static void BleAppHandleGattServiceRead(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    UINT8 attErr = 0;
    UINT16 attrid = ind->handle - gGattSvc->startHdl;
    BLE_APP_PRINT("BleAppHandleGattServiceRead attId = %d offset = %d\r\n", attrid, ind->offset);
    switch (attrid)
    {
        case GATT_IDX_SERVICE_CHANGE_CFG:
        break;

        default:
            attErr = LE_ATT_ERR_READ_NOT_PERMITTED;
        break;
    }

    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, attErr);
}

static void BleAppHandleGattServiceWrite(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    UINT8 attErr = 0;
    UINT16 attrid = ind->handle - gGattSvc->startHdl;
    BLE_APP_PRINT("BleAppHandleGattServiceWrite attId = %d op = %x offset = %d\r\n", attrid, ind->flag, ind->offset);
    switch (attrid)
    {
        case GATT_IDX_SERVICE_CHANGE_CFG:
        {
            UINT16 val = *((UINT16 *)ind->pVal);
            
            BLE_APP_PRINT("GATT_IDX_SERVICE_CHANGE_CFG Len = %d  attr-val = %d\r\n", val);

            LeGattChangeAttrVal(gGattSvc, attrid, 2, &val);
        }
        break;

        default:
            attErr = LE_ATT_ERR_WRITE_NOT_PERMITTED;
        break;
    }
    
    LeGattAccessWriteRsp(ind->conn_hdl, ind->flag, ind->handle, attErr);
}

static void BleAppHandleGapServiceRead(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    UINT8 attErr = 0;
    UINT16 attrid = ind->handle - gGapSvc->startHdl;
    BLE_APP_PRINT("BleAppHandleGapServiceRead called\n\r");    
    switch (attrid)
    {
        case GAP_IDX_DEVICE_NAME_VAL:
        {
            BLE_APP_PRINT("GAP_IDX_DEVICE_NAME_VAL offset = %d\r\n", ind->offset);
        }
        break;
        
        default:
            attErr = LE_ATT_ERR_READ_NOT_PERMITTED;
        break;
    }
    
    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, attErr);
}

static void BleAppHandleGapServiceWrite(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    UINT8 attErr = 0;
    UINT16 attrid = ind->handle - gGapSvc->startHdl;
    BLE_APP_PRINT("BleAppHandleGapServiceWrite attId = %d op = %x offset = %d\r\n", attrid, ind->flag, ind->offset);
    switch (attrid)
    {
        case GAP_IDX_DEVICE_NAME_VAL:
        {
            if (ind->offset > 31)
            {
                attErr = LE_ATT_ERR_INVALID_OFFSET;
            }
            else if ((ind->offset + ind->len) > 31)
            {
                attErr = LE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            else
            {
                // It could change or modify the attribute value, because the length of device name is variable length. (max length is not equal zero.)
                LeGattChangeAttrVal(gGapSvc, attrid, ind->len, ind->pVal);
            }
        }
        break;

        default:
            attErr = LE_ATT_ERR_WRITE_NOT_PERMITTED;
        break;
    }

    LeGattAccessWriteRsp(ind->conn_hdl, ind->flag, ind->handle, attErr);
}

static void BleAppHandleBlpsServiceRead(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
    UINT8 attErr = 0;
    UINT16 attrid = ind->handle - gBlpsSvc->startHdl;
    BLE_APP_PRINT("BleAppHandleBlpsServiceRead attId = %d offset = %d\r\n", attrid, ind->offset);
    switch (attrid)
    {
        case BWP_IDX_DATA_OUT_CFG:
        case BWP_IDX_DATA_IN_VAL:
        break;

        default:
            attErr = LE_ATT_ERR_READ_NOT_PERMITTED;
        break;
    }

    LeGattAccessReadRsp(ind->conn_hdl, ind->handle, attErr);
}

static void BleAppHandleBlpsServiceWrite(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    UINT8 attErr = 0;
    UINT16 attrid = ind->handle - gBlpsSvc->startHdl;
    BLE_APP_PRINT("BleAppHandleBlpsServiceWrite attId = %d op = %x offset = %d\r\n", attrid, ind->flag, ind->offset);
    switch (attrid)
    {
        case BWP_IDX_DATA_OUT_CFG:
        case BWP_IDX_DATA_IN_VAL:
        {
            LeGattChangeAttrVal(gBlpsSvc, attrid, 2, ind->pVal);
        }
        break;

        default:
            attErr = LE_ATT_ERR_WRITE_NOT_PERMITTED;
        break;
    }
    
    LeGattAccessWriteRsp(ind->conn_hdl, ind->flag, ind->handle, attErr);
}

static void BleGattHandleAccessRead(LE_GATT_MSG_ACCESS_READ_IND_T *ind)
{
     BLE_APP_PRINT("BleGattHandleAccessRead called\n\r");    
    if ((ind->handle >= gGattSvc->startHdl) && (ind->handle <= gGattSvc->endHdl))
    {
        BleAppHandleGattServiceRead(ind);
    }
    else if ((ind->handle >= gGapSvc->startHdl) && (ind->handle <= gGapSvc->endHdl))
    {
        BleAppHandleGapServiceRead(ind);
    }
    else if ((ind->handle >= gBlpsSvc->startHdl) && (ind->handle <= gBlpsSvc->endHdl))
    {
        BleAppHandleBlpsServiceRead(ind);
    }
    else
    {
        LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_READ_NOT_PERMITTED);
    }
}

static void BleGattHandleAccessWrite(LE_GATT_MSG_ACCESS_WRITE_IND_T *ind)
{
    BLE_APP_PRINT("BleGattHandleAccessWrite called\n\r");    
    if ((ind->handle >= gGattSvc->startHdl) && (ind->handle <= gGattSvc->endHdl))
    {
        BleAppHandleGattServiceWrite(ind);
    }
    else if ((ind->handle >= gGapSvc->startHdl) && (ind->handle <= gGapSvc->endHdl))
    {
        BleAppHandleGapServiceWrite(ind);
    }
    else if ((ind->handle >= gBlpsSvc->startHdl) && (ind->handle <= gBlpsSvc->endHdl))
    {
        BleAppHandleBlpsServiceWrite(ind);
    }
    else
    {
        LeGattAccessReadRsp(ind->conn_hdl, ind->handle, LE_ATT_ERR_WRITE_NOT_PERMITTED);
    }
}

void BleAppGattMsgHandler(TASK task, MESSAGEID id, MESSAGE message)
{
    BLE_APP_PRINT("BleAppGattMsgHandler called\n\r"); 
    switch (id)
    {
        case LE_GATT_MSG_INIT_CFM:
        {
            BLE_APP_PRINT("LE_GATT_MSG_INIT_CFM\r\n");

            gGattSvc = LeGattRegisterService(gGattSvcDb, sizeof(gGattSvcDb) / sizeof(LE_GATT_ATTR_T));

            if (gGattSvc)
            {
                BLE_APP_PRINT("LeGattRegisterService gGattSvc success\r\n");
            }

            gGapSvc = LeGattRegisterService(gGapSvcDb, sizeof(gGapSvcDb) / sizeof(LE_GATT_ATTR_T));

            if (gGapSvc)
            {
                BLE_APP_PRINT("LeGattRegisterService gGapSvc success\r\n");
            }

            gBlpsSvc = LeGattRegisterService(gBwpSvcDb, sizeof(gBwpSvcDb) / sizeof(LE_GATT_ATTR_T));

            if (gBlpsSvc)
            {
                BLE_APP_PRINT("LeGattRegisterService gBlpsSvc success\r\n");
            }
        }
        break;

        case LE_GATT_MSG_ACCESS_READ_IND:
        {
            BLE_APP_PRINT("LE_GATT_MSG_ACCESS_READ_IND\r\n");
            BleGattHandleAccessRead((LE_GATT_MSG_ACCESS_READ_IND_T *)message);
        }
        break;

        case LE_GATT_MSG_ACCESS_WRITE_IND:
        {
            BLE_APP_PRINT("LE_GATT_MSG_ACCESS_WRITE_IND\r\n");
            BleGattHandleAccessWrite((LE_GATT_MSG_ACCESS_WRITE_IND_T *)message);
        }
        break;

        case LE_GATT_MSG_EXCHANGE_MTU_IND:
        {
            LE_GATT_MSG_EXCHANGE_MTU_IND_T *ind = (LE_GATT_MSG_EXCHANGE_MTU_IND_T *)message;
            BLE_APP_PRINT("LE_GATT_MSG_EXCHANGE_MTU_IND client mtu = %d\r\n", ind->client_rx_mtu);
            LeGattExchangeMtuRsp(ind->conn_hdl, LE_ATT_MAX_MTU);
        }
        break;

        case LE_GATT_MSG_EXCHANGE_MTU_CFM:
        {
            LE_GATT_MSG_EXCHANGE_MTU_CFM_T *cfm = (LE_GATT_MSG_EXCHANGE_MTU_CFM_T *)message;
            BLE_APP_PRINT("LE_GATT_MSG_EXCHANGE_MTU_CFM curr mtu = %d\r\n", cfm->current_rx_mtu);
            BleAppGetEntity()->curr_mtu = cfm->current_rx_mtu;
        }
        break;

        case LE_GATT_MSG_CONFIRMATION_CFM:
        {
            LE_GATT_MSG_CONFIRMATION_CFM_T *cfm = (LE_GATT_MSG_CONFIRMATION_CFM_T *)message;
            BLE_APP_PRINT("LE_GATT_MSG_CONFIRMATION_CFM curr handle = %d\r\n", cfm->handle);

        }
        break;

        case LE_GATT_MSG_OPERATION_TIMEOUT:
        {
            LE_GATT_MSG_OPERATION_TIMEOUT_T *ind = (LE_GATT_MSG_OPERATION_TIMEOUT_T *)message;
            BLE_APP_PRINT("LE_GATT_MSG_OPERATION_TIMEOUT op = %x\r\n", ind->att_op);
        }
        break;

        default:
        break;
    }
}

void BleGattIndicateServiceChange(UINT16 conn_hdl)
{
    UINT16 len;
    UINT16 val;
    BLE_APP_PRINT("BleGattIndicateServiceChange called\n\r"); 
    LE_ERR_STATE rc = LeGattGetAttrVal(gGattSvc, GATT_IDX_SERVICE_CHANGE_CFG, &len, &val);
    
    if (rc) return;

    if (val == LE_GATT_CLIENT_CFG_INDICATION)
    {
        UINT16 handle[2];

        LeGattGetAttrVal(gGattSvc, GATT_IDX_SERVICE_CHANGE_VAL, &len, handle);

        if (!handle[0] || !handle[1]) return;

        LeGattCharValIndicate(conn_hdl, LeGattGetAttrHandle(gGattSvc, GATT_IDX_SERVICE_CHANGE_VAL), 4, (UINT8 *)handle);
    }
}


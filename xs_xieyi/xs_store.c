// Sec 0: Comment block of the file

// Sec 1: Include File
#include "mw_fim.h"
#include "mw_fim_default.h"
#include "xs_app.h"

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

typedef enum
{
    MW_FIM_GP12_START = 0x01020000,  /*  01 Zone, 02 Group, File ID */
    MW_FIM_GP12_SENSOR_DATA,
    MW_FIM_GP12_MAX
} E_MwFimIdxGroup12_Extension;



// the group version number
#define MW_FIM_VER12_Extension        0x04


#define MW_FIM_ZONE1_BASE_ADDR_Ex  0x00090000 /* Physical Memory allocation */
#define MW_FIM_ZONE1_BLOCK_SIZE_Ex 0x1000
#define MW_FIM_ZONE1_BLOCK_NUM_Ex  3          /* 2 Group BLOCK + SWAP BLOCK = 3  BLOCK */

/********************************************
Declaration of data structure
********************************************/
// Sec 3: structure, uniou, enum, linked list



#define MW_FIM_SENSOR_DATA_CFG_SIZE   sizeof(xs_app_data_t)
#define MW_FIM_SENSOR_DATA_CFG_NUM    10

/********************************************
Declaration of Global Variables & Functions
********************************************/
// Sec 4: declaration of global variable

const T_MwFimFileInfo g_taMwFimGroupTable12_Extension[];
/* The default value of sensor data number */
const xs_app_data_t g_MwFimDefaultSensorDataNumber = {};
/* The address buffer of sensor data number */
uint32_t g_ulaMwFimAddrBufferConfig[MW_FIM_SENSOR_DATA_CFG_NUM];

// the information table of group 12
const T_MwFimFileInfo g_taMwFimGroupTable12_Extension[] = 
{
    {MW_FIM_GP12_SENSOR_DATA, MW_FIM_SENSOR_DATA_CFG_NUM, MW_FIM_SENSOR_DATA_CFG_SIZE, (uint8_t*)&g_MwFimDefaultSensorDataNumber, g_ulaMwFimAddrBufferConfig},
    
    {0xFFFFFFFF, 0x00, 0x00, NULL, NULL}
};

// Sec 5: declaration of global function prototype

/***************************************************
Declaration of static Global Variables & Functions
***************************************************/
// Sec 6: declaration of static global variable

// Sec 7: declaration of static function prototype

/***********
C Functions
***********/
// Sec 8: C Functions
void xs_store_init()
{
	// update here
    T_MwFimZoneInfo taMwFimZoneInfoTable_Extension =
    {
        MW_FIM_ZONE1_BASE_ADDR_Ex,
        MW_FIM_ZONE1_BLOCK_SIZE_Ex,
        MW_FIM_ZONE1_BLOCK_NUM_Ex,
        g_ubaMwFimVersionTable[1]
    };

    // add the new zone 01
    MwFim_ZoneInfoUpdate(1, &taMwFimZoneInfoTable_Extension);
    
    // update the group information
    MwFim_GroupInfoUpdate(1, 2, (T_MwFimFileInfo *)g_taMwFimGroupTable12_Extension);
    MwFim_GroupVersionUpdate(1, 2, MW_FIM_VER12_Extension);
	
}

void get_store_data(xs_app_data_t *tmp)
{
	/* Group 12 Sensor Data */
    if (MW_FIM_OK != MwFim_FileRead(MW_FIM_GP12_SENSOR_DATA, 6, MW_FIM_SENSOR_DATA_CFG_SIZE, (uint8_t *)tmp))
    {
        // if fail, give the default value
        printf("\r\nGroup 12 Read Number Value ERROR\r\n");
    } 
    else
    {
        printf("\r\nGroup 12 Read Number = %d from FIM\r\n",tmp->check_sum);
    }
}
void set_store_data(xs_app_data_t *tmp)
{
	 if (MW_FIM_OK != MwFim_FileWrite(MW_FIM_GP12_SENSOR_DATA, 6, MW_FIM_SENSOR_DATA_CFG_SIZE, (uint8_t *)tmp))
    {
        printf("\r\nGroup 12 Write Number Value ERROR\r\n");
    } 
    else
    {
        printf("\r\nGroup 12 Writ Number = %d into FIM\r\n",tmp->check_sum);
    }
}


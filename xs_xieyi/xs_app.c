// #include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "sys_init_patch.h"
#include "hal_system.h"
#include "mw_fim.h"
#include "cmsis_os.h"
#include "sys_os_config.h"
#include "at_cmd_common_patch.h"
#include "hal_pin.h"
#include "hal_pin_def.h"
#include "hal_dbg_uart.h"
#include "hal_vic.h"
#include "boot_sequence.h"
#include "at_cmd_common_patch.h"
#include "hal_uart.h"

#include "xs_app.h"

#define APP_MESSAGE_Q_SIZE  16

#define DEFAULT_WIFI_SSID "Testing"
#define DEFAULT_WIFI_PASSWORD "senssun@2019"
#define DEFAULT_BLE_ADV_NAME "SENSSUN02"
#define DEFAULT_SERVER "114.67.206.79"
#define DEFAULT_SERVER_PORT 8082
#define DEFAULT_DEVICE_ID  ""


static osThreadId g_tAppThread_1;
static osMessageQId g_tAppMessageQ;
static osPoolId g_tAppMemPoolId;

xs_app_data_t g_app_data={};

extern void get_store_data(xs_app_data_t *tmp);
extern void set_store_data(xs_app_data_t *tmp);
static void Main_AppThread_1(void *argu);

void set_wifi_para(char* ssid,char* psd)
{
	strcpy(g_app_data.wifi_ssid,ssid);
	strcpy(g_app_data.wifi_password,psd);
    set_store_data(&g_app_data);
}
void set_device_id(char* dev_id)
{
	memcpy(g_app_data.device_id,dev_id,11);
	set_store_data(&g_app_data);
}
void set_server_para(char* server,int port)
{
	strcpy(g_app_data.server,server);
	g_app_data.server_port=port;
	set_store_data(&g_app_data);
}
void set_ble_adv_name(char* adv_name)
{
	strcpy(g_app_data.ble_adv_name,adv_name);
	set_store_data(&g_app_data);
}

void init_app()
{
	osThreadDef_t tThreadDef;
    osMessageQDef_t tMessageDef;
    osPoolDef_t tMemPoolDef;

	extern void xs_uart_init(); 
    extern void xs_wifi_init(void);
    extern void BleWifi_Ble_ServerAppInit(void); 
    extern void xs_ble_init(char* dev_id);

	char *D=g_app_data.device_id;
	get_store_data(&g_app_data);
	if(g_app_data.check_sum!=3456)
	{
		printf("device not init!\n");
		char default_device_id[11]={
			0x00,0x01,
			0x01,
			0x00,0x00,
			0x11,0x22,0x33,0x44,0x55,0x66
		};

		strcpy(g_app_data.wifi_ssid,DEFAULT_WIFI_SSID);
		strcpy(g_app_data.wifi_password,DEFAULT_WIFI_PASSWORD);
		strcpy(g_app_data.server,DEFAULT_SERVER);
		strcpy(g_app_data.ble_adv_name,DEFAULT_BLE_ADV_NAME);
		g_app_data.server_port=DEFAULT_SERVER_PORT;

    	memcpy(g_app_data.device_id,default_device_id,11);
	}


	printf("device_id:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
		D[0],D[1],D[2],D[3],D[4],D[5],D[6],D[7],D[8],D[9],D[10] );


	xs_uart_init();
    //BT INIT
    xs_ble_init(g_app_data.device_id);
    //
    xs_wifi_init();
    //
  
	  //OtaAppInit();
    //
    // create the thread for AppThread_1
    tThreadDef.name = "App_1";
    tThreadDef.pthread = Main_AppThread_1;
    tThreadDef.tpriority = OS_TASK_PRIORITY_APP;        // osPriorityNormal
    tThreadDef.instances = 0;                           // reserved, it is no used
    tThreadDef.stacksize = OS_TASK_STACK_SIZE_APP;      // (512), unit: 4-byte, the size is 512*4 bytes
    g_tAppThread_1 = osThreadCreate(&tThreadDef, NULL);
    if (g_tAppThread_1 == NULL)
    {
        printf("To create the thread for AppThread_1 is fail.\n");
    }

    // create the message queue for AppMessageQ
    tMessageDef.queue_sz = APP_MESSAGE_Q_SIZE;          // number of elements in the queue
    tMessageDef.item_sz = sizeof(S_MessageQ);           // size of an item
    tMessageDef.pool = NULL;                            // reserved, it is no used
    g_tAppMessageQ = osMessageCreate(&tMessageDef, g_tAppThread_1);
    if (g_tAppMessageQ == NULL)
    {
        printf("To create the message queue for AppMessageQ is fail.\n");
    }
    
    // create the memory pool for AppMessageQ
    tMemPoolDef.pool_sz = APP_MESSAGE_Q_SIZE;           // number of items (elements) in the pool
    tMemPoolDef.item_sz = sizeof(S_MessageQ);           // size of an item
    tMemPoolDef.pool = NULL;                            // reserved, it is no used
    g_tAppMemPoolId = osPoolCreate(&tMemPoolDef);
    if (g_tAppMemPoolId == NULL)
    {
        printf("To create the memory pool for AppMessageQ is fail.\n");
    }


}

/*************************************************************************
* FUNCTION:
*   Main_AppThread_1
*
* DESCRIPTION:
*   the application thread 1
*
* PARAMETERS
*   1. argu     : [In] the input argument
*
* RETURNS
*   none
*
*************************************************************************/
static void Main_AppThread_1(void *argu)
{
    osEvent tEvent;
    S_MessageQ *ptMsgPool;
    uint32_t ulCount = 0;
    extern void xs_send_from_ble(char* buff,int len);

    while (1)
    {

    	ulCount++;
        printf("xs_v1.0.5 free heap:%d,sys_running:%ldms\n",xPortGetFreeHeapSize(),xTaskGetTickCount());

        xs_send_from_ble("12345678",8);
        tEvent = osMessageGet(g_tAppMessageQ, 10000);
        if (tEvent.status != osEventMessage)
        {
            printf("To receive the message from AppMessageQ is fail.\n");
            continue;
        }
        
        // get the content of message
        ptMsgPool = (S_MessageQ *)tEvent.value.p;        
        // output the contect of message
        
        printf("Hello world %d\n", ptMsgPool->type);  
        switch( ptMsgPool->type)
        {

            case 1:
                printf("uart msg\n");

                break;
            case 2:
                printf("ble msg\n");

                break;
            case 3:
                printf("wifi msg\n");

                break;
            default:

                break;
        }      
        // free the memory pool
        osPoolFree(g_tAppMemPoolId, ptMsgPool);
        
      

        
    }
}
void send_msg_to_app(char type,char* buff)
{
    S_MessageQ *ptMsgPool;
    
    // allocate the memory pool
    ptMsgPool = (S_MessageQ *)osPoolCAlloc(g_tAppMemPoolId);
    if (ptMsgPool == NULL)
    {
        printf("To allocate the memory pool for AppMessageQ is fail.\n");
       	return;
    }
    
    // copy the message content
    ptMsgPool->type=type;
    memcpy(ptMsgPool->buff, buff, buff[4]);
    
    // send the message
    if (osOK != osMessagePut(g_tAppMessageQ, (uint32_t)ptMsgPool, osWaitForever))
    {
        printf("To send the message for AppMessageQ is fail.\n");        
        // free the memory pool
        osPoolFree(g_tAppMemPoolId, ptMsgPool);
    }
    

	return;
}

void print_hex( char *pbSrc, int nLen)
{
    char ddl,ddh;
    int i;
    char pbDest[200];

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
    printf("%s\n", pbDest);
}
char creat_check_code(char* buff)
{
    int i=0,sum=0,len=buff[4]-1;
    for(i=4;i<len;i++)
    {
        sum+=buff[i];
    }
    sum &=0xFF;

    return sum;
}
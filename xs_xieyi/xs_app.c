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
// #define DEFAULT_WIFI_SSID "Zoujq-iphone11"
// #define DEFAULT_WIFI_PASSWORD "zz123456"

#define DEFAULT_BLE_ADV_NAME "SENSSUN02"
#define DEFAULT_SERVER "114.67.206.79"
#define DEFAULT_SERVER_PORT 8023
#define DEFAULT_DEVICE_ID  ""


static osThreadId g_tAppThread_1;
static osMessageQId g_tAppMessageQ;
static osPoolId g_tAppMemPoolId;

xs_app_data_t g_app_data={};
char g_packet_buff[200];
extern char g_ble_connect_status;
char *D=g_app_data.device_id;

extern void get_store_data(xs_app_data_t *tmp);
extern void set_store_data(xs_app_data_t *tmp);
static void Main_AppThread_1(void *argu);
extern void xs_send_from_tcp(char* buff,int len);
extern void xs_send_from_ble(char* buff,int len);
extern void xs_send_from_uart(char* buff,int len);
extern void xs_ota_start(char* url);
extern void xs_uart_init(); 
extern void xs_wifi_init(void);
extern void xs_ble_init(char* dev_id);
extern void xs_tcp_init(char* server,int port);
extern void change_user_ssid_psd(char* ssid,char* psd);

extern bool g_wifi_connection_flag;
extern char g_tcp_connect_status;
extern char g_ble_connect_status;

void send_tcp_login();
void send_tcp_ping();
void uart_opration(char* packet);
void ble_opration(char* packet);
void tcp_opration(char* packet);



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

	
	get_store_data(&g_app_data);
	if(g_app_data.check_sum!=3457)
	{
		printf("device not init!\n");
		char default_device_id[11]={
            0x91,0x54 ,
            0x01 ,0x01 ,0x15 ,
            0xAB ,0xAB ,0xAB ,0xAB,0xAB ,0x01
		};
        g_app_data.check_sum=3456;

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
    xs_tcp_init(g_app_data.server,g_app_data.server_port);
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


    while (1)
    {

    	ulCount++;
        printf("%s free heap:%d,sys_running:%ldms,wifi:%d,tcp:%d,ble:%d\n",
            VERSION,
            xPortGetFreeHeapSize(),
            xTaskGetTickCount(),
            g_wifi_connection_flag,
            g_tcp_connect_status,
            g_ble_connect_status);

        // send_tcp_login();
        tEvent = osMessageGet(g_tAppMessageQ, 10000);
        if (tEvent.status != osEventMessage)
        {
            printf("To receive the message from AppMessageQ is fail.\n");
            send_tcp_ping();
            // xs_send_from_ble("12345678",8);
            // xs_send_from_uart("12345678",8);
            continue;
        }
        
        // get the content of message
        ptMsgPool = (S_MessageQ *)tEvent.value.p;        
        // output the contect of message

        switch( ptMsgPool->type)
        {

            case 1:
                printf("uart msg\n");
                uart_opration(ptMsgPool->buff);
                break;
            case 2:
                printf("ble msg\n");
                ble_opration(ptMsgPool->buff);
                break;
            case 3:
                printf("tcp msg\n");
                tcp_opration(ptMsgPool->buff);
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

void create_packet(char f1,char f2,char* data,int len)
{
    int all_len=len+8;
    g_packet_buff[0]=0x10;
    g_packet_buff[1]=0x00;
    g_packet_buff[2]=0x00;
    g_packet_buff[3]=0xC5;
    g_packet_buff[4]=all_len;
    g_packet_buff[5]=f1;
    g_packet_buff[6]=f2;
    memcpy(g_packet_buff+7,data,len);
    g_packet_buff[all_len-1]=creat_check_code(g_packet_buff);
}

void send_tcp_login(){

    create_packet(0x99,0x99,g_app_data.device_id,11);
    xs_send_from_tcp(g_packet_buff,g_packet_buff[4]);
}
void send_tcp_ping(){

    create_packet(0x99,0x98,g_app_data.device_id,0);
    xs_send_from_tcp(g_packet_buff,g_packet_buff[4]);
}


void uart_opration(char* packet)
{
    if(packet[5]==0xaa && packet[6]==0x80)
    {
        char state[1]={0};
        state[0]=g_tcp_connect_status*4+g_ble_connect_status*2;
        create_packet(0xaa,0x00,state,1);
        xs_send_from_uart(g_packet_buff,g_packet_buff[4]);
    }
    else
    {
        xs_send_from_ble(packet,packet[4]);
        xs_send_from_tcp(packet,packet[4]);
    }
}
void ble_opration(char* packet)
{
    if(packet[5]==0xaa)
    {
        switch(packet[6])
        {
            case 0x1b:
            {
                static char url[100]={0};
                memcpy(url,packet+7,packet[4]-8);
                url[packet[4]-8]=0;
                printf("start ota,load from url:%s\n", url);
                xs_ota_start(url);
                break;  
            }              
            case 0x1c:
                memcpy(g_app_data.wifi_ssid,packet+8,packet[7]);
                memcpy(g_app_data.wifi_password,packet+8+1+packet[7],packet[8+1+packet[7]]);
                printf("set wifi :%s,%s\n", g_app_data.wifi_ssid,g_app_data.wifi_password);
                //set_store_data(&g_app_data);
                change_user_ssid_psd(g_app_data.wifi_ssid,g_app_data.wifi_password);
                break;
            case 0x1d:
                {
                    char buff1[65];
                    char ssid_len=strlen(g_app_data.wifi_ssid);
                    char psw_len=strlen(g_app_data.wifi_password);

                    buff1[0]=ssid_len;
                    buff1[ssid_len+1]=psw_len;
                    memcpy(buff1+1,g_app_data.wifi_ssid,ssid_len);
                    memcpy(buff1+1+ssid_len +1,g_app_data.wifi_password,psw_len);
                    create_packet(0xaa,0x1d,buff1,2+ssid_len+psw_len);
                    xs_send_from_ble(g_packet_buff,g_packet_buff[4]);
                    break;
                }
            case 0x1e:
                memcpy(g_app_data.server,packet+8,packet[7]);
                g_app_data.server_port=packet[8+1+packet[7]]<<8 +packet[8+2+packet[7]];
                printf("set server :%s,%d\n", g_app_data.server,g_app_data.server_port);
                set_store_data(&g_app_data);
                break;
            case 0x20:
                memcpy(g_app_data.device_id,packet+7,packet[4]-8);
                printf("set device_id:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
        D[0],D[1],D[2],D[3],D[4],D[5],D[6],D[7],D[8],D[9],D[10] );
                set_store_data(&g_app_data);
                break;
            case 0x21:
                create_packet(0xaa,0x21,g_app_data.device_id,11);
                xs_send_from_ble(g_packet_buff,g_packet_buff[4]);
                break;
            case 0x81:
                memcpy(g_app_data.ble_adv_name,packet+7,packet[4]-8);
                set_store_data(&g_app_data);
                break;
            default:
                break;
        }
    }
    else
    {
        xs_send_from_uart(packet,packet[4]);
    }
}
void tcp_opration(char* packet)
{
    if(packet[5]==0x00 && packet[6]==0x99)
    {
        if(packet[7]==0)
        {
            printf("login success\n");
        }
        else
        {
            printf("login error\n");
        }
    }
    else if(packet[5]==0x00 && packet[6]==0x98)
    {
        printf("receive pong\n");
    }
    else
    {
         xs_send_from_uart(packet,packet[4]);
    }
   
}
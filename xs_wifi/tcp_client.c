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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "event_loop.h"
#include "wifi_api.h"
#include "wifi_event.h"
#include "wifi_event_handler.h"
#include "lwip_helper.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "errno.h"

#include "ps_public.h"
#include "msg_patch.h"

// #define  TCP_SERVER_ADDR    "192.168.17.109" 
// #define  TCP_SERVER_PORT    9000

#define  DTIM_SKIP_COUNT    29
#define  POWER_SAVE_EN      1

#define  TCP_RECV_TIMEOUT    180

osThreadId tcp_client_task_id;

extern bool g_wifi_connection_flag ;
extern void send_tcp_login();

static uint32_t g_ulRcvCount = 0;
void xs_tcp_received_cb(char* buff,int len);

char g_tcp_connect_status=0;
char rev_buff[301];
int s;
char* g_server=0;
int g_port=0;
void tcp_thread(void *args)
{
    /*
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
  */        
    struct sockaddr_in serverAdd;  
    char recv_buf[256];
    int r;

    lwip_net_ready();

    osDelay(500);
        
    serverAdd.sin_family = AF_INET; 
    serverAdd.sin_addr.s_addr = inet_addr(g_server);  
    serverAdd.sin_port = htons(g_port); 

    printf("Connect %s at port %d \r\n", g_server,g_port); 
        
    while (1) {
        if(g_wifi_connection_flag==0)
        {
            osDelay(500);
            continue;
        }
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            printf("... Failed to allocate socket. \r\n");
            osDelay(1000);
            continue;
        }
        printf("... allocated socket \r\n");

        if (connect(s, (struct sockaddr *)&serverAdd, sizeof(serverAdd)) != 0) {
            printf("... socket connect failed errno=%d \r\n", errno);
            close(s);
            osDelay(4000);
            continue;
        }

        printf("... connected \r\n");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = TCP_RECV_TIMEOUT;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            printf("... failed to set socket receiving timeout \r\n");
            close(s);
            osDelay(3000);
            continue;
        }
        g_tcp_connect_status=1;
        send_tcp_login();
        printf("... set socket receiving timeout success \r\n");
        do {
            memset(recv_buf, 0, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            
            if (r > 0) {
                g_ulRcvCount++;
                printf("Rcv %u\n", g_ulRcvCount);
            
                xs_tcp_received_cb(recv_buf,r);
              
            }
        } while (r > 0);
        close(s);
        g_tcp_connect_status=0;
        printf("Starting again! \r\n");
        osDelay(2000);
    }
}


void xs_tcp_init(char* server,int port)
{
    osThreadDef_t task_def;
   
    g_server=server;
    g_port=port;
    /* Create task */
    task_def.name = "user_app";
    task_def.stacksize = OS_TASK_STACK_SIZE_APP;
    task_def.tpriority = OS_TASK_PRIORITY_APP;
    task_def.pthread = tcp_thread;
    tcp_client_task_id = osThreadCreate(&task_def, (void*)NULL);

    if(tcp_client_task_id == NULL)
    {
        printf("user_app Task create fail \r\n");
    }
    else
    {
        printf("user_app Task create successful \r\n");
    }
}



void xs_tcp_received_cb(char* buff,int len)
{
    static int receive_counter=0;
    static uint32_t last_tick=0;

    if(receive_counter>0)
    {
        if(xTaskGetTickCount()-last_tick>500)
        {
            receive_counter=0;
        }
    }
    
    if(receive_counter+len>300)
    {
        receive_counter=0;
        return;
    }    
    memcpy(rev_buff+receive_counter,buff,len);
    receive_counter+=len;
    printf("tcp rd:");
    print_hex(rev_buff,receive_counter);
    if(receive_counter>5)
    {
        if(rev_buff[0]==0x10 && rev_buff[1]==0x00 && rev_buff[2]==0x00 && rev_buff[3]==0xC5)
        {
            if(rev_buff[4]<=receive_counter)
            {
                send_msg_to_app(3,rev_buff);
                receive_counter=0;
            }
            else
            {
                receive_counter+=len;
            }
        }
        else
        {
            receive_counter=0;
        }
    }
    
    last_tick=xTaskGetTickCount();
}
void xs_send_from_tcp(char* buff,int len)
{
    if(g_tcp_connect_status==1)
    {
        if (s<0 ||  write(s, buff, len) < 0) 
        {
            printf("... socket send failed \r\n");
            close(s);
            g_tcp_connect_status=0;
        }
        {
            printf("tcp send:");
            print_hex(buff,len);
        }
    }
    else
    {
        printf("tcp send faild ,wifi not connet\n");
    }
   
    
}
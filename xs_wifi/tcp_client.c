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

#define  TCP_SERVER_ADDR    "192.168.17.109" 
#define  TCP_SERVER_PORT    9000

#define  DTIM_SKIP_COUNT    29
#define  POWER_SAVE_EN      1

#define  TCP_RECV_TIMEOUT    180

osThreadId tcp_client_task_id;

extern bool g_connection_flag ;

static uint32_t g_ulRcvCount = 0;

void tcp_thread(void *args)
{
    /*
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
  */        
    int s;
    struct sockaddr_in serverAdd;  
    char server_ip[32];
    int server_port = TCP_SERVER_PORT; 
    char recv_buf[256];
    int r;

    lwip_net_ready();

    osDelay(500);
        
    serverAdd.sin_family = AF_INET; 
        serverAdd.sin_addr.s_addr = inet_addr(TCP_SERVER_ADDR);  
        serverAdd.sin_port = htons(TCP_SERVER_PORT); 

        strcpy(server_ip,TCP_SERVER_ADDR);
        
      if (g_connection_flag == true) 
          printf("Opulinks-TEST-AP connected \r\n");
    
        printf("Connect %s at port %d \r\n", server_ip,server_port); 
        
    while (1) {
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
            osDelay(4000);
            continue;
        }
        printf("... set socket receiving timeout success \r\n");
        do {
            memset(recv_buf, 0, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            
            if (r > 0) {
                g_ulRcvCount++;
                printf("Rcv %u\n", g_ulRcvCount);
            
                if (write(s, recv_buf, strlen(recv_buf)) < 0) {
                    printf("... socket send failed \r\n");
                    close(s);
                    osDelay(4000);
                    continue;
                }
            }
        } while (r > 0);
        close(s);

        printf("Starting again! \r\n");
    }
}


void xs_tcp_init(void)
{
    osThreadDef_t task_def;
   

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


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sys_init.h"
#include "sys_init_patch.h"
#include "hal_system.h"
#include "mw_fim.h"
#include "cmsis_os.h"
#include "sys_os_config.h"
#include "Hal_pinmux_uart.h"
#include "hal_pin.h"
#include "hal_pin_def.h"
//#include "hal_pin_config_project.h"
#include "at_cmd_common_patch.h"
#include "hal_dbg_uart.h"
#include "hal_vic.h"
#include "boot_sequence.h"
#include "hal_uart.h"
#include "xs_app.h"



char rev_buff[100];
static void xs_uart_received_cb(uint32_t ulData)
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
	rev_buff[receive_counter]=0xFF & ulData;
	if(receive_counter>5)
	{
		if(rev_buff[0]==0x10 && rev_buff[1]==0x00 && rev_buff[2]==0x00 && rev_buff[3]==0xC5)
		{
			if(rev_buff[4]==receive_counter+1)
			{
				send_msg_to_app(1,rev_buff);
				receive_counter=0;
				printf("uart received:");
				print_hex(rev_buff,rev_buff[4]);
			}
			else
			{
				receive_counter++;
			}
		}
		else
		{
			receive_counter=0;
		}
	}
	else
	{
		receive_counter++;
	}
	
	last_tick=xTaskGetTickCount();
	
}
void xs_send_from_uart(char* buff,int len)
{
	int i=0;
	for(i=0;i<len;i++){
		Hal_Uart_DataSend(UART_IDX_0,buff[i]);
	}
	printf("uart send:");
	print_hex(rev_buff,len);
}
//	6
void xs_uart_init()
{
	Hal_PinMux_Uart_Init(&OPL1000_periph.uart[0]);

	   // set the rx callback function and enable the rx interrupt
    Hal_Uart_RxCallBackFuncSet(UART_IDX_0, xs_uart_received_cb);
    Hal_Uart_RxIntEn(UART_IDX_0, 1);
}
#include "ble_gatt_if.h"
#include "ble_uuid.h"
#include "xs_app.h"

#include "ble_app.h"
#include "ble_app_gatt.h"

char g_ble_connect_status=0;
char rev_buff[301];
void xs_ble_received_cb(char* buff,int len)
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
	print_hex(rev_buff,receive_counter);
	if(receive_counter>5)
	{
		if(rev_buff[0]==0x10 && rev_buff[1]==0x00 && rev_buff[2]==0x00 && rev_buff[3]==0xC5)
		{
			if(rev_buff[4]<=receive_counter)
			{
				send_msg_to_app(2,rev_buff);
				receive_counter=0;
				
			}
			else
			{
				receive_counter+=len;
			}
		}
		else
		{
			printf("packet header error");
			receive_counter=0;
		}
	}
	
	last_tick=xTaskGetTickCount();
}
void xs_send_from_ble(char* buff,int len)
{
	if(g_ble_connect_status){
		BleWifi_Ble_SendAppMsgToBle(BLEWIFI_APP_MSG_SEND_DATA, len, buff);
		printf("ble send:");
		print_hex(buff,len);
	}
	else{
		printf("ble send fail ,not connect:");
	}
	
}
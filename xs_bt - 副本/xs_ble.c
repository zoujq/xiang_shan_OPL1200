
#include "ble_gatt_if.h"
#include "ble_uuid.h"
#include "xs_app.h"

char rev_buff[200];
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
	memcpy(rev_buff+receive_counter,buff,len);
	receive_counter+=len;
	if(receive_counter>5)
	{
		if(rev_buff[0]==0x10 && rev_buff[1]==0x00 && rev_buff[2]==0x00 && rev_buff[3]==0xC5)
		{
			if(rev_buff[4]<=receive_counter)
			{
				send_msg_to_app(2,rev_buff);
				receive_counter=0;
				printf("ble received:");
				print_hex(rev_buff,rev_buff[4]);
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
void xs_send_from_ble(char* buff,int len)
{

}
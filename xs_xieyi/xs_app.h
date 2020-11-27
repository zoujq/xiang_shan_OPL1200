#ifndef __XS_APP_H__
#define __XS_APP_H__

typedef struct {
    int check_sum;
    char wifi_ssid[25];
    char wifi_password[25];
    char ble_adv_name[31];
    char server[30];
    int server_port;
    char device_id[11];
}xs_app_data_t;

typedef struct
{
    char type;
    char buff[100];
} S_MessageQ;
void send_msg_to_app(char type,char* buff);
void print_hex( char *pbSrc, int nLen);

#endif  // end of __XS_APP_H__
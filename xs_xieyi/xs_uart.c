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

static void xs_uart_received_cb(uint32_t ulData)
{
	printf("%d\n", ulData);
	Hal_Uart_DataSend(UART_IDX_0,'Y');
}
void xs_init_uart()
{
	Hal_PinMux_Uart_Init(&OPL1000_periph.uart[0]);

	   // set the rx callback function and enable the rx interrupt
    Hal_Uart_RxCallBackFuncSet(UART_IDX_0, xs_uart_received_cb);
    Hal_Uart_RxIntEn(UART_IDX_0, 1);
}
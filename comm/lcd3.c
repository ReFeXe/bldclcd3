#include "commands.h"
#include "ch.h"
#include "hal.h"
#include "mc_interface.h"
#include "stm32f4xx_conf.h"
#include "buffer.h"
#include "terminal.h"
#include "hw.h"
#include "app.h"
#include "timeout.h"
#include "utils_sys.h"
#include "packet.h"
#include "qmlui.h"
#include "crc.h"
#include "main.h"
#include "conf_custom.h"

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


extern void app_uartcomm_send_raw_packet(unsigned char *data, unsigned int len, UART_PORT port_number);

#define LCD3_REPLY_PACKET_SIZE	12

#define MOVING_ANIMATION_THROTTLE	(1 << 1)
#define MOVING_ANIMATION_CRUISE		(1 << 3)
#define MOVING_ANIMATION_ASSIST		(1 << 4)
#define MOVING_ANIMATION_BRAKE		(1 << 5)

void lcd3_process_packet(unsigned char *data, unsigned int len,
		void(*reply_func)(unsigned char *data, unsigned int len))
{
	(void)len;
	
	uint8_t lcd_pas_mode = 0;
	lcd_pas_mode = data[1];
	
	float current_scale;
	
	if (lcd_pas_mode == 1)
		current_scale = 0.1;
	else if (lcd_pas_mode > 1)
		current_scale = (lcd_pas_mode / 6);
	
	
	app_pas_set_current_sub_scaling(current_scale);
	
	
	
	uint8_t sb[LCD3_REPLY_PACKET_SIZE];
	
	int32_t ms = mc_interface_get_speed();
	
	uint8_t batteryLevel;
	uint8_t batFlashing = 0;

	float l = mc_interface_get_battery_level(NULL);
	
	
	if (l > 0.7)
		batteryLevel = 4;
	else if (l > 0.4)
		batteryLevel = 3;
	else if (l > 0.2)
		batteryLevel = 2;
	else if (l > 0.1)
		batteryLevel = 1;
	else
	{
		batteryLevel = 0;
		if (l <= 0)
			batFlashing = 1;
	}
	
	
	float w = (float)GET_INPUT_VOLTAGE() * mc_interface_read_reset_avg_input_current() / 12;
	if (w < 0)
		w = 0;
	if (w > 255)
		w = 255;
	
	sb[0] = 0x41;
	
	//b1: battery level:
	// bit 0: border flashing,
	// bit 1: animated charging,
	// bit 3-5: level, (0-4)
	sb[1] = (batteryLevel << 2) | batFlashing;
	
	sb[2] = 0x30;
	sb[3] = (ms >> 8) & 0xff;	//b3: speed, wheel rotation period, ms; period(ms)=B3*256+B4;
	sb[4] = (ms >> 0) & 0xff;	//b4:
	sb[5] = 0;	//b5: B5 error info display: 0x20: "0info", 0x21: "6info", 0x22: "1info", 0x23: "2info", 0x24: "3info", 0x25: "0info", 0x26: "4info", 0x28: "0info"
	sb[6] = 0;
	
	//b7: moving animation ()
	// bit 0: -
	// bit 1: throttle
	// bit 2: -
	// bit 3: cruise
	// bit 4: assist
	// bit 5: brake
	// bit 6: -
	// bit 7: -
	sb[7] = 
		((app_adc_get_decoded_level() > 0) ? MOVING_ANIMATION_THROTTLE : 0) |
		(0) |
		((app_pas_get_pas_level() > 0) ? MOVING_ANIMATION_ASSIST : 0) |
		((app_adc_get_decoded_level2() > 0) ? MOVING_ANIMATION_BRAKE : 0);
	
	sb[8] = w;	//b8: power in 13 wt increments (48V version of the controller)
	sb[9] = (int8_t)(mc_interface_temp_motor_filtered() - 15.0f);	//b9: motor temperature +15
	sb[10] = 0;	//
	sb[11] = 0;	//
	
	uint8_t crc = 0;
	for (int n = 1; n < LCD3_REPLY_PACKET_SIZE; n++)
		crc ^= sb[n];
	
	sb[6] = crc;
	
	app_uartcomm_send_raw_packet(sb,len,UART_PORT_COMM_HEADER);
}

#define LCD3_RX_PACKET_SIZE	13
static uint8_t buffer[LCD3_RX_PACKET_SIZE];

void lcd3_process_byte(uint8_t rx_data, PACKET_STATE_t *state)
{
	(void)state;
	
	memmove(buffer, &buffer[1], LCD3_RX_PACKET_SIZE - 1);
	buffer[LCD3_RX_PACKET_SIZE - 1] = rx_data;
	
	if (buffer[12] == 0x0e)
	{
		lcd3_process_packet(buffer, LCD3_RX_PACKET_SIZE, UART_PORT_COMM_HEADER);
	}
}

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
	(void)reply_func;
	
	volatile mc_configuration *mcconf = (volatile mc_configuration*) mc_interface_get_configuration();
	



	
	uint8_t lcd_pas_mode = data[1];  //speedbutton
	bool fixed_throttle_level = (data[4] >> 4) & 1;  //p4
	bool temp_mode =  (data[10] >> 2) & 1;  //c13
	bool l3 =  (data[10] >> 0) & 1;
	
	float current_scale = 0.0;
	
	if (lcd_pas_mode == 1)
		current_scale = 0.1;
	else if (lcd_pas_mode == 2)
		current_scale = 0.2;
	else if (lcd_pas_mode == 3)
		current_scale = 0.35;
	else if (lcd_pas_mode == 4)
		current_scale = 0.65;
	else if (lcd_pas_mode == 5)
		current_scale = 1;
		
		
	
	if(fixed_throttle_level == 0) {
		mcconf->l_current_max_scale = 1.0;
		app_pas_set_current_sub_scaling(current_scale);
	} else {
		mcconf->l_current_max_scale = current_scale;
	}
	
	if((current_scale == 0.0) && l3) {
		mcconf->l_current_max_scale = current_scale;
	}
	
	
	uint8_t sb[LCD3_REPLY_PACKET_SIZE];
	
	
	float ms = (float)(mc_interface_get_speed()* 3600.0 / 1000.0); //speed vesc to km.h
	
	
	uint16_t pms = 0;
	if (ms < 2){
	 pms = 0;
	} else {
	 pms = (7360 / ms);
	}
	
	
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
	
	
	int8_t temp;
	if(temp_mode) {
		temp = (int8_t)mc_interface_temp_motor_filtered();	
	} else {
		temp = (int8_t)mc_interface_temp_fet_filtered();
	}
	
	//b1: battery level:
	// bit 0: border flashing,
	// bit 1: animated charging,
	// bit 3-5: level, (0-4)
	sb[1] = (batteryLevel << 2) | batFlashing;
	
	sb[2] = 0x30; //battery voltage
	
	sb[3] = (uint8_t)(pms/256);	//b3, b4: speed, wheel rotation period, ms; period(ms)=B3*256+B4;
	sb[4] = pms - sb[3]*256;
	
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
		((app_pas_get_pedal_rpm() > 1) ? MOVING_ANIMATION_ASSIST : 0) |
		((app_adc_get_decoded_level2() > 0) ? MOVING_ANIMATION_BRAKE : 0);
	
	sb[8] = w;	//b8: power in 13 wt increments (48V version of the controller)

	sb[9] = (int8_t)(temp - 15.0f);	//b9: motor temperature +15
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

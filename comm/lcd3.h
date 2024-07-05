/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef LCD3_H_
#define LCD3_H_


//is reducer motor difined speed external hall sensor
#define HW_HAS_WHEEL_SPEED_SENSOR

#include "datatypes.h"

void lcd3_process_packet(unsigned char *data, unsigned int len,
		void(*reply_func)(unsigned char *data, unsigned int len));
float hw_get_speed(void);
void hw_update_speed_sensor(void);
float hw_get_distance(void);
float hw_get_distance_abs(void);

#endif /* LCD3_H_ */
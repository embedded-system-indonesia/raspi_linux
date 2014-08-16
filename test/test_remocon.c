/* 
 * Copyright (C) 2014, Siryogi Majdi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details at 
 * <http://www.gnu.org/licenses/>.
 */


#include <string.h>			// memset, memcpy
#include <stdio.h>			// printf, snprintf
#include <unistd.h>			// usleep, sleep
#include "gpio.h"
#include "remocon.h"


static struct gpio_class gpio;
static uint32_t port_led_1 = 9;
static uint32_t port_led_2 = 10;


static void rmc_callback(uint8_t *data_rcv, uint8_t len)
{
	if (data_rcv == NULL || len == 0)
		return;

	if (data_rcv[0] == 0x1C && data_rcv[1] == 0x10) {
		// Numeric 1 pressed, LED 1 ON
		gpio.set_level(port_led_1, 1);
		gpio.set_level(port_led_2, 0);
	}
	else if (data_rcv[0] == 0x1C && data_rcv[1] == 0x11) {
		// Numeric 2 pressed, LED 2 ON
		gpio.set_level(port_led_1, 0);
		gpio.set_level(port_led_2, 1);
	}
	else if (data_rcv[0] == 0x1C && data_rcv[1] == 0x12) {
		// Numeric 3 pressed, All LED OFF
		gpio.set_level(port_led_1, 0);
		gpio.set_level(port_led_2, 0);
	}
}


int main()
{
	rmc_desc_t rd;
	uint32_t port_ir = 18;  // Infra red port number
	
	// Create remocon descriptor
	if ((rd = rmc_new(RMC_FORMAT_KASEIKYO, port_ir, rmc_callback)) == RMC_DESC_ERR)
		return -1;

	// Create GPIO object
	if (gpio_new(&gpio) < 0)
		return -1;

	gpio.set_mode(port_led_1, GPIO_MODE_OUTPUT);
	gpio.set_mode(port_led_2, GPIO_MODE_OUTPUT);

	// Loop forever, can add timeout if necessary
	while (1) {
		sleep(1);
	}
	
	rmc_free(rd);
	gpio_free();
	
	return 0;
}


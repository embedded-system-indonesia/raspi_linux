/* 
 * I2C communication controller
 * 
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

#include <unistd.h>			// usleep
#include <string.h>			// memset, memcpy
#include <stdio.h>			// printf, snprintf
#include "gpio.h"
#include "i2c.h"


#define I2C_DESC_MAX			(20)	// Support up to 20 I2C line
#define I2C_BIT_DELAY_HIGH		(2)  	// 2 usec
#define I2C_BIT_DELAY_FULL		(5)  	// 5 usec
#define BW(v)					(1 << (v))


struct _i2c_desc_buf_ {
	i2c_type_t 		type;
	i2c_speed_t		speed;
	uint32_t 		port_scl;
	uint32_t 		port_sda;
	uint8_t 		used;
};

struct _i2c_info_ {
	struct gpio_class	gpio_obj;
};

static struct _i2c_desc_buf_ i2c_desc_buf[I2C_DESC_MAX];
static struct _i2c_info_ i2c_info;


static i2c_desc_t i2c_get_desc(void)
{
	i2c_desc_t desc = I2C_DESC_ERR;

	for (desc = 0; desc < I2C_DESC_MAX; desc++) {
		if (i2c_desc_buf[desc].used == 0)
			break;
	}

	if (desc >= I2C_DESC_MAX)
		return I2C_DESC_ERR;

	return desc;
}


static int i2c_start_comm_soft_mst(struct _i2c_desc_buf_ *desc_buf, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in)
{
	uint32_t bit_delay = (desc_buf->speed == I2C_SPEED_HIGH) ? I2C_BIT_DELAY_HIGH : I2C_BIT_DELAY_FULL;
	uint32_t ctdat = 0;
	uint8_t  ctbit = 0;
	struct gpio_class *gpio_obj = &i2c_info.gpio_obj;
	uint8_t error = 0;

	// Set mode port
	gpio_obj->set_mode(desc_buf->port_sda, GPIO_MODE_OUTPUT);
	gpio_obj->set_mode(desc_buf->port_scl, GPIO_MODE_OUTPUT);

	// Start bit
	gpio_obj->set_level(desc_buf->port_scl, 1);
	gpio_obj->set_level(desc_buf->port_sda, 1);
	usleep(bit_delay);
	gpio_obj->set_level(desc_buf->port_sda, 0);
	usleep(bit_delay);
	gpio_obj->set_level(desc_buf->port_scl, 0);
	usleep(bit_delay);

	// Send data
	for (ctdat = 0; ctdat < len_out; ctdat++) {
		gpio_obj->set_mode(desc_buf->port_sda, GPIO_MODE_OUTPUT);
		
		for (ctbit = 0; ctbit < 8; ctbit++) {
			if (data_out[ctdat] & BW(7 - ctbit))
				gpio_obj->set_level(desc_buf->port_sda, 1);
			else
				gpio_obj->set_level(desc_buf->port_sda, 0);				

			usleep(bit_delay);
			gpio_obj->set_level(desc_buf->port_scl, 1);
			usleep(bit_delay);
			gpio_obj->set_level(desc_buf->port_scl, 0);
		}

		gpio_obj->set_mode(desc_buf->port_sda, GPIO_MODE_INPUT);
		usleep(bit_delay);
		gpio_obj->set_level(desc_buf->port_scl, 1);

		// Check ACK
		if (gpio_obj->get_level(desc_buf->port_sda)) {
			// NACK
			printf("<error i2c_start_comm_soft_mst> NACK, data no = %d, value = 0x%02x\n", ctdat, data_out[ctdat]);
			error = 1;
			break;
		}

		usleep(bit_delay);
		gpio_obj->set_level(desc_buf->port_scl, 0);
	}

	// Read data
	if (!error && len_in > 0 && data_in) {
		for (ctdat = 0; ctdat < len_in; ctdat++) {
			gpio_obj->set_mode(desc_buf->port_sda, GPIO_MODE_INPUT);
			
			for (ctbit = 0; ctbit < 8; ctbit++) {
				usleep(bit_delay);
				gpio_obj->set_level(desc_buf->port_scl, 1);
				usleep(bit_delay);
				
				if (gpio_obj->get_level(desc_buf->port_sda))
					data_in[ctdat] |= BW(7 - ctbit);
				else
					data_in[ctdat] &= ~(BW(7 - ctbit));

				gpio_obj->set_level(desc_buf->port_scl, 0);
			}		

			// Set ACK / NACK
			gpio_obj->set_mode(desc_buf->port_sda, GPIO_MODE_OUTPUT);
			if (ctdat < (len_in - 1))
				gpio_obj->set_level(desc_buf->port_sda, 0);
			else
				gpio_obj->set_level(desc_buf->port_sda, 1);
			
			usleep(bit_delay);
			gpio_obj->set_level(desc_buf->port_scl, 1);
			usleep(bit_delay);
			gpio_obj->set_level(desc_buf->port_scl, 0);
		}	
	}
	
	// Stop bit
	gpio_obj->set_mode(desc_buf->port_sda, GPIO_MODE_OUTPUT);
	usleep(bit_delay);
	gpio_obj->set_level(desc_buf->port_scl, 1);
	usleep(bit_delay);
	gpio_obj->set_level(desc_buf->port_sda, 1);
	
	return error ? -1 : 0;
}


static int i2c_start_comm(i2c_desc_t i2c_desc, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in)
{
	struct _i2c_desc_buf_ *desc_buf = &i2c_desc_buf[i2c_desc];
	int ret = -1;

	if (i2c_desc >= I2C_DESC_MAX || desc_buf->used == 0 || data_out == NULL || len_out == 0)
		return -1;
	
	switch (desc_buf->type) {
	case I2C_TYPE_SOFT_MASTER:
		ret = i2c_start_comm_soft_mst(desc_buf, data_out, len_out, data_in, len_in);
		break;
	}
	
	return ret;
}


i2c_desc_t i2c_new(struct i2c_class *i2c_obj, i2c_type_t type, i2c_speed_t speed, uint32_t port_scl, uint32_t port_sda)
{
	i2c_desc_t desc;
	struct _i2c_desc_buf_ *desc_buf;
	
	if (i2c_obj == NULL || port_scl == I2C_PORT_UNUSED)
		return I2C_DESC_ERR;

	if ((desc = i2c_get_desc()) == I2C_DESC_ERR)
		return I2C_DESC_ERR;

	desc_buf = &i2c_desc_buf[desc];

	// Load GPIO
	if (gpio_new(&i2c_info.gpio_obj) < 0)
		return I2C_DESC_ERR;

	desc_buf->type     = type;
	desc_buf->speed    = speed;
	desc_buf->port_scl = port_scl;
	desc_buf->port_sda = port_sda;
	desc_buf->used     = 1;
	
	i2c_obj->start_comm = i2c_start_comm;

	return desc;
}


int i2c_free(i2c_desc_t i2c_desc)
{
	if (i2c_desc >= I2C_DESC_MAX || i2c_desc_buf[i2c_desc].used == 0) 
		return -1;

	memset(&i2c_desc_buf[i2c_desc], 0, sizeof(struct _i2c_desc_buf_));
	
	return 0;
}



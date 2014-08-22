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


#define OBJ_BUF_SIZE			(20)	// Support up to 20 I2C line
#define I2C_BIT_DELAY_HIGH		(2)  	// 2 usec
#define I2C_BIT_DELAY_FULL		(5)  	// 5 usec
#define BW(v)					(1 << (v))


typedef struct {
	i2c_type_t 		type;
	i2c_speed_t		speed;
	int 			port_scl;
	int		 		port_sda;
	uint8_t 		used;
} i2c_obj_buf_t;

typedef struct {
	gpio_class_t	gpio_obj;
} i2c_info_t;

static i2c_obj_buf_t obj_buf_list[OBJ_BUF_SIZE];
static i2c_info_t    i2c_info;


static int i2c_get_obj_buf_id(void)
{
	int id = -1;

	for (id = 0; id < OBJ_BUF_SIZE; id++) {
		if (obj_buf_list[id].used == 0)
			break;
	}

	if (id >= OBJ_BUF_SIZE)
		return -1;

	return id;
}


static i2c_obj_buf_t *i2c_get_obj_buf(int id)
{
	i2c_obj_buf_t *obj_buf = &obj_buf_list[id];

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return NULL;

	return obj_buf;
}


static int i2c_start_comm_soft_mst(int id, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in)
{
	i2c_obj_buf_t *obj_buf  = i2c_get_obj_buf(id);
	gpio_class_t  *gpio_obj = &i2c_info.gpio_obj;
	uint32_t bit_delay = (obj_buf->speed == I2C_SPEED_HIGH) ? I2C_BIT_DELAY_HIGH : I2C_BIT_DELAY_FULL;
	uint32_t ctdat = 0;
	uint8_t  ctbit = 0;
	uint8_t  error = 0;

	// Set mode port
	gpio_obj->set_mode(obj_buf->port_sda, GPIO_MODE_OUTPUT);
	gpio_obj->set_mode(obj_buf->port_scl, GPIO_MODE_OUTPUT);

	// Start bit
	gpio_obj->set_level(obj_buf->port_scl, 1);
	gpio_obj->set_level(obj_buf->port_sda, 1);
	usleep(bit_delay);
	gpio_obj->set_level(obj_buf->port_sda, 0);
	usleep(bit_delay);
	gpio_obj->set_level(obj_buf->port_scl, 0);
	usleep(bit_delay);

	// Send data
	for (ctdat = 0; ctdat < len_out; ctdat++) {
		gpio_obj->set_mode(obj_buf->port_sda, GPIO_MODE_OUTPUT);
		
		for (ctbit = 0; ctbit < 8; ctbit++) {
			if (data_out[ctdat] & BW(7 - ctbit))
				gpio_obj->set_level(obj_buf->port_sda, 1);
			else
				gpio_obj->set_level(obj_buf->port_sda, 0);				

			usleep(bit_delay);
			gpio_obj->set_level(obj_buf->port_scl, 1);
			usleep(bit_delay);
			gpio_obj->set_level(obj_buf->port_scl, 0);
		}

		gpio_obj->set_mode(obj_buf->port_sda, GPIO_MODE_INPUT);
		usleep(bit_delay);
		gpio_obj->set_level(obj_buf->port_scl, 1);

		// Check ACK
		if (gpio_obj->get_level(obj_buf->port_sda)) {
			// NACK
			printf("<error i2c_start_comm_soft_mst> NACK, data no = %d, value = 0x%02x\n", ctdat, data_out[ctdat]);
			error = 1;
			break;
		}

		usleep(bit_delay);
		gpio_obj->set_level(obj_buf->port_scl, 0);
	}

	// Read data
	if (!error && len_in > 0 && data_in) {
		for (ctdat = 0; ctdat < len_in; ctdat++) {
			gpio_obj->set_mode(obj_buf->port_sda, GPIO_MODE_INPUT);
			
			for (ctbit = 0; ctbit < 8; ctbit++) {
				usleep(bit_delay);
				gpio_obj->set_level(obj_buf->port_scl, 1);
				usleep(bit_delay);
				
				if (gpio_obj->get_level(obj_buf->port_sda))
					data_in[ctdat] |= BW(7 - ctbit);
				else
					data_in[ctdat] &= ~(BW(7 - ctbit));

				gpio_obj->set_level(obj_buf->port_scl, 0);
			}		

			// Set ACK / NACK
			gpio_obj->set_mode(obj_buf->port_sda, GPIO_MODE_OUTPUT);
			if (ctdat < (len_in - 1))
				gpio_obj->set_level(obj_buf->port_sda, 0);
			else
				gpio_obj->set_level(obj_buf->port_sda, 1);
			
			usleep(bit_delay);
			gpio_obj->set_level(obj_buf->port_scl, 1);
			usleep(bit_delay);
			gpio_obj->set_level(obj_buf->port_scl, 0);
		}	
	}
	
	// Stop bit
	gpio_obj->set_mode(obj_buf->port_sda, GPIO_MODE_OUTPUT);
	usleep(bit_delay);
	gpio_obj->set_level(obj_buf->port_scl, 1);
	usleep(bit_delay);
	gpio_obj->set_level(obj_buf->port_sda, 1);
	
	return error ? -1 : 0;
}


static int i2c_start_comm(int id, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in)
{
	i2c_obj_buf_t *obj_buf  = i2c_get_obj_buf(id);
	int ret = -1;

	if (obj_buf == NULL || data_out == NULL || len_out == 0)
		return -1;
	
	switch (obj_buf->type) {
	case I2C_TYPE_SOFT_MASTER:
		ret = i2c_start_comm_soft_mst(id, data_out, len_out, data_in, len_in);
		break;
	}
	
	return ret;
}


int i2c_new(i2c_class_t *i2c_obj, i2c_type_t type, i2c_speed_t speed, int port_scl, int port_sda)
{
	int id;
	i2c_obj_buf_t *obj_buf;
	
	if (i2c_obj == NULL || port_scl < 0)
		return -1;

	if ((id = i2c_get_obj_buf_id()) < 0)
		return -1;

	obj_buf = &obj_buf_list[id];

	// Load GPIO
	if (gpio_new(&i2c_info.gpio_obj) < 0)
		return -1;

	obj_buf->type     = type;
	obj_buf->speed    = speed;
	obj_buf->port_scl = port_scl;
	obj_buf->port_sda = port_sda;
	obj_buf->used     = 1;
	
	i2c_obj->start_comm = i2c_start_comm;

	return id;
}


int i2c_free(int id)
{
	i2c_obj_buf_t *obj_buf = i2c_get_obj_buf(id);

	if (obj_buf == NULL)
		return -1;

	memset(&obj_buf_list[id], 0, sizeof(i2c_obj_buf_t));
	
	return 0;
}



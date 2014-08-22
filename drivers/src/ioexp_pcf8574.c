/* 
 * IO Expander PCF8574
 * 8 bit IO contolled using I2C
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

#include <string.h>				// memset, memcpy
#include <stdio.h>				// printf, snprintf
#include "i2c.h"
#include "ioexp_pcf8574.h"


#define OBJ_BUF_SIZE				(10)
#define I2C_SLAVE_ADDR				(0x27)
#define BW(v)						(1 << (v))


typedef struct {
	i2c_class_t 	i2c;
	int       		i2c_id;
	uint8_t     	ports;	
	uint8_t 		used;
} ioexp_obj_buf_t;


static ioexp_obj_buf_t obj_buf_list[OBJ_BUF_SIZE];


static int ioexp_get_obj_buf_id(void)
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


static ioexp_obj_buf_t *ioexp_get_obj_buf(int id)
{
	ioexp_obj_buf_t *obj_buf = &obj_buf_list[id];

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return NULL;

	return obj_buf;
}


int ioexp_set_all_port(int id, int ports)
{
	ioexp_obj_buf_t *obj_buf = ioexp_get_obj_buf(id);
	uint8_t data_out[2];
	uint8_t len_out;

	if (obj_buf == NULL)
		return -1;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	data_out[1] = (uint8_t)ports;
	len_out     = 2;

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, NULL, 0) < 0)
		return -1;

	obj_buf->ports = data_out[1];

	return 0;	
}


uint32_t ioexp_get_all_port(int id)
{
	ioexp_obj_buf_t *obj_buf = ioexp_get_obj_buf(id);
	uint8_t data_out[1];
	uint8_t len_out;
	uint8_t data_in[1];
	uint8_t len_in;

	if (obj_buf == NULL)
		return -1;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 1;

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, data_in, len_in) < 0)
		return 0;

	obj_buf->ports = data_in[0];

	return (uint32_t)obj_buf->ports;	
}


int ioexp_set_one_port(int id, int port, uint8_t level)
{
	ioexp_obj_buf_t *obj_buf = ioexp_get_obj_buf(id);
	uint8_t data_out[2];
	uint8_t len_out;

	if (obj_buf == NULL || port > 8)
		return -1;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 2;
	if (level)
		data_out[1] = obj_buf->ports | BW(port);
	else
		data_out[1] = obj_buf->ports & ~(BW(port));

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, NULL, 0) < 0)
		return -1;

	obj_buf->ports = data_out[1];

	return 0;	
}


uint8_t ioexp_get_one_port(int id, int port)
{
	ioexp_obj_buf_t *obj_buf = ioexp_get_obj_buf(id);
	uint8_t data_out[1];
	uint8_t len_out;
	uint8_t data_in[1];
	uint8_t len_in;
	uint8_t ret;

	if (obj_buf == NULL || port > 8)
		return 0;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 1;

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, data_in, len_in) < 0)
		return 0;

	obj_buf->ports = data_in[0];
	ret = obj_buf->ports & BW(port) ? 1 : 0;

	return ret;	
}


int ioexp_pcf8574_new(iopcf_class_t *ioexp_obj, int port_scl, int port_sda)
{
	int id;
	ioexp_obj_buf_t *obj_buf;

	if (ioexp_obj == NULL || port_scl < 0)
		return -1;

	if ((id = ioexp_get_obj_buf_id()) < 0)
		return -1;

	obj_buf = &obj_buf_list[id];

	// Create I2C object
	if ((obj_buf->i2c_id = i2c_new(&obj_buf->i2c, I2C_TYPE_SOFT_MASTER, I2C_SPEED_FULL, port_scl, port_sda)) < 0)
		return -1;

	ioexp_obj->set_all_port = ioexp_set_all_port;
	ioexp_obj->get_all_port = ioexp_get_all_port;
	ioexp_obj->set_one_port = ioexp_set_one_port;
	ioexp_obj->get_one_port = ioexp_get_one_port;

	// Mark as used
	obj_buf->used = 1;

	return id;
}


int ioexp_pcf8574_free(int id)
{
	ioexp_obj_buf_t *obj_buf = ioexp_get_obj_buf(id);

	if (obj_buf == NULL)
		return -1;
	
	// Delete I2C object
	i2c_free(obj_buf->i2c_id);

	// Clear buffer
	memset(obj_buf, 0, sizeof(ioexp_obj_buf_t));
	
	return 0;
}



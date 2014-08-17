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


struct _ioexp_obj_buf_ {
	struct i2c_class 	i2c;
	i2c_id_t       		i2c_id;
	uint8_t       		ports;	
	uint8_t 			used;
};


static struct _ioexp_obj_buf_ obj_buf_list[OBJ_BUF_SIZE];


static int ioexp_pcf8574_get_obj_buf_id(void)
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



int ioexp_pcf8574_set_all_port(ioexp_pcf8574_id_t id, uint32_t ports)
{
	struct _ioexp_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t data_out[2];
	uint8_t len_out;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0)
		return -1;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	data_out[1] = (uint8_t)ports;
	len_out     = 2;

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, NULL, 0) < 0)
		return -1;

	obj_buf->ports = data_out[1];

	return 0;	
}


uint32_t ioexp_pcf8574_get_all_port(ioexp_pcf8574_id_t id)
{
	struct _ioexp_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t data_out[1];
	uint8_t len_out;
	uint8_t data_in[1];
	uint8_t len_in;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0)
		return 0;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 1;

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, data_in, len_in) < 0)
		return 0;

	obj_buf->ports = data_in[0];

	return (uint32_t)obj_buf->ports;	
}


int ioexp_pcf8574_set_one_port(ioexp_pcf8574_id_t id, uint32_t port, uint8_t level)
{
	struct _ioexp_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t data_out[2];
	uint8_t len_out;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0 || port > 8)
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


uint8_t ioexp_pcf8574_get_one_port(ioexp_pcf8574_id_t id, uint32_t port)
{
	struct _ioexp_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t data_out[1];
	uint8_t len_out;
	uint8_t data_in[1];
	uint8_t len_in;
	uint8_t ret;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0 || port > 8)
		return 0;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 1;

	if (obj_buf->i2c.start_comm(obj_buf->i2c_id, data_out, len_out, data_in, len_in) < 0)
		return 0;

	obj_buf->ports = data_in[0];
	ret = obj_buf->ports & BW(port) ? 1 : 0;

	return ret;	
}


ioexp_pcf8574_id_t ioexp_pcf8574_new(struct ioexp_pcf8574_class *ioexp_obj, uint32_t port_scl, uint32_t port_sda)
{
	ioexp_pcf8574_id_t id;
	struct _ioexp_obj_buf_ *obj_buf;

	if (ioexp_obj == NULL || port_scl == IOEXP_PCF8574_PORT_UNUSED)
		return IOEXP_PCF8574_ID_ERR;

	if ((id = ioexp_pcf8574_get_obj_buf_id()) < 0)
		return IOEXP_PCF8574_ID_ERR;

	obj_buf = &obj_buf_list[id];

	// Create I2C object
	if ((obj_buf->i2c_id = i2c_new(&obj_buf->i2c, I2C_TYPE_SOFT_MASTER, I2C_SPEED_FULL, port_scl, port_sda)) == I2C_ID_ERR)
		return IOEXP_PCF8574_ID_ERR;

	ioexp_obj->set_all_port = ioexp_pcf8574_set_all_port;
	ioexp_obj->get_all_port = ioexp_pcf8574_get_all_port;
	ioexp_obj->set_one_port = ioexp_pcf8574_set_one_port;
	ioexp_obj->get_one_port = ioexp_pcf8574_get_one_port;

	// Mark as used
	obj_buf->used = 1;

	return id;
}


int ioexp_pcf8574_free(ioexp_pcf8574_id_t id)
{
	struct _ioexp_obj_buf_ *obj_buf = &obj_buf_list[id];

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return -1;
	
	// Delete I2C object
	i2c_free(obj_buf->i2c_id);

	// Clear buffer
	memset(obj_buf, 0, sizeof(struct _ioexp_obj_buf_));
	
	return 0;
}



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


#define DESC_BUF_SIZE				(10)
#define I2C_SLAVE_ADDR				(0x27)
#define BW(v)						(1 << (v))


struct _desc_buf_ {
	struct i2c_class 	i2c;
	i2c_desc_t       	i2c_desc;
	uint8_t       		ports;	
	uint8_t 			used;
};


static struct _desc_buf_ desc_buf[DESC_BUF_SIZE];


static int ioexp_pcf8574_get_desc(void)
{
	int desc = -1;
	
	for (desc = 0; desc < DESC_BUF_SIZE; desc++) {
		if (desc_buf[desc].used == 0)
			break;
	}

	if (desc >= DESC_BUF_SIZE)
		return -1;

	return desc;
}


int ioexp_pcf8574_set_all_port(ioexp_pcf8574_desc_t desc, uint32_t ports)
{
	struct _desc_buf_ *pdesc = &desc_buf[desc];
	uint8_t data_out[2];
	uint8_t len_out;

	if (desc >= DESC_BUF_SIZE || pdesc->used == 0)
		return -1;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	data_out[1] = (uint8_t)ports;
	len_out     = 2;

	if (pdesc->i2c.start_comm(pdesc->i2c_desc, data_out, len_out, NULL, 0) < 0)
		return -1;

	pdesc->ports = data_out[1];

	return 0;	
}


uint32_t ioexp_pcf8574_get_all_port(ioexp_pcf8574_desc_t desc)
{
	struct _desc_buf_ *pdesc = &desc_buf[desc];
	uint8_t data_out[1];
	uint8_t len_out;
	uint8_t data_in[1];
	uint8_t len_in;

	if (desc >= DESC_BUF_SIZE || pdesc->used == 0)
		return 0;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 1;

	if (pdesc->i2c.start_comm(pdesc->i2c_desc, data_out, len_out, data_in, len_in) < 0)
		return 0;

	pdesc->ports = data_in[0];

	return (uint32_t)pdesc->ports;	
}


int ioexp_pcf8574_set_one_port(ioexp_pcf8574_desc_t desc, uint32_t port, uint8_t level)
{
	struct _desc_buf_ *pdesc = &desc_buf[desc];
	uint8_t data_out[2];
	uint8_t len_out;

	if (desc >= DESC_BUF_SIZE || pdesc->used == 0 || port > 8)
		return -1;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 2;
	if (level)
		data_out[1] = pdesc->ports | BW(port);
	else
		data_out[1] = pdesc->ports & ~(BW(port));

	if (pdesc->i2c.start_comm(pdesc->i2c_desc, data_out, len_out, NULL, 0) < 0)
		return -1;

	pdesc->ports = data_out[1];

	return 0;	
}


uint8_t ioexp_pcf8574_get_one_port(ioexp_pcf8574_desc_t desc, uint32_t port)
{
	struct _desc_buf_ *pdesc = &desc_buf[desc];
	uint8_t data_out[1];
	uint8_t len_out;
	uint8_t data_in[1];
	uint8_t len_in;
	uint8_t ret;

	if (desc >= DESC_BUF_SIZE || pdesc->used == 0 || port > 8)
		return 0;

	data_out[0] = I2C_SLAVE_ADDR << 1 | 0;
	len_out     = 1;

	if (pdesc->i2c.start_comm(pdesc->i2c_desc, data_out, len_out, data_in, len_in) < 0)
		return 0;

	pdesc->ports = data_in[0];
	ret = pdesc->ports & BW(port) ? 1 : 0;

	return ret;	
}


ioexp_pcf8574_desc_t ioexp_pcf8574_new(struct ioexp_pcf8574_class *ioexp_obj, uint32_t port_scl, uint32_t port_sda)
{
	ioexp_pcf8574_desc_t desc;
	struct _desc_buf_ *pdesc;

	if (ioexp_obj == NULL || port_scl == IOEXP_PCF8574_PORT_UNUSED)
		return IOEXP_PCF8574_DESC_ERR;

	if ((desc = ioexp_pcf8574_get_desc()) < 0)
		return IOEXP_PCF8574_DESC_ERR;

	pdesc = &desc_buf[desc];

	// Create I2C object
	if ((pdesc->i2c_desc = i2c_new(&pdesc->i2c, I2C_TYPE_SOFT_MASTER, I2C_SPEED_FULL, port_scl, port_sda)) == I2C_DESC_ERR)
		return IOEXP_PCF8574_DESC_ERR;

	ioexp_obj->set_all_port = ioexp_pcf8574_set_all_port;
	ioexp_obj->get_all_port = ioexp_pcf8574_get_all_port;
	ioexp_obj->set_one_port = ioexp_pcf8574_set_one_port;
	ioexp_obj->get_one_port = ioexp_pcf8574_get_one_port;

	// Mark as used
	pdesc->used = 1;

	return desc;
}


int ioexp_pcf8574_free(ioexp_pcf8574_desc_t ioexp_pcf8574_desc)
{
	struct _desc_buf_ *pdesc = &desc_buf[ioexp_pcf8574_desc];

	if (ioexp_pcf8574_desc >= DESC_BUF_SIZE || pdesc->used == 0) 
		return -1;
	
	// Delete I2C object
	i2c_free(pdesc->i2c_desc);

	// Clear buffer
	memset(pdesc, 0, sizeof(struct _desc_buf_));
	
	return 0;
}



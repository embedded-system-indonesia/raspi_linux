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
#include "spi.h"
#include "tm1638.h"


#define OBJ_BUF_SIZE		(10)


struct _tm1638_obj_buf_ {
	tm1638_type_t    	type;
	struct spi_class 	spi;
	spi_id_t       		spi_id;
	tm1638_pulse_t   	pulse;
	uint8_t         	display_on;
	uint8_t 		   	used;
};

static struct _tm1638_obj_buf_ obj_buf_list[OBJ_BUF_SIZE];


static int tm1638_get_obj_buf_id(void)
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


static int tm1638_display_on(tm1638_id_t id, uint8_t on_off)
{
	struct _tm1638_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t buf[1];
	uint8_t buf_len;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return -1;

	buf[0]  = 0x80;
	buf[0] |= on_off ? 8 : 0;
	buf[0] |= obj_buf->pulse & 7;

	if (obj_buf->spi.start_comm(obj_buf->spi_id, buf, 1, NULL, 0) < 0)
		return -1;

	obj_buf->display_on = on_off;
	
	return 0;
}


static int tm1638_pulse_width(tm1638_id_t id, tm1638_pulse_t pulse)
{
	struct _tm1638_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t buf[1];
	uint8_t buf_len;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return -1;

	buf[0]  = 0x80;
	buf[0] |= obj_buf->display_on ? 8 : 0;
	buf[0] |= pulse & 7;

	if (obj_buf->spi.start_comm(obj_buf->spi_id, buf, 1, NULL, 0) < 0)
		return -1;

	obj_buf->pulse = pulse;
	
	return 0;
}


static int tm1638_write_display(tm1638_id_t id, const union tm1638_format *data_out)
{
	struct _tm1638_obj_buf_ *obj_buf = &obj_buf_list[id];
	uint8_t buf[17];
	uint8_t buf_len;
	uint8_t seg, grid;

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0 || data_out == NULL) 
		return -1;

	// Set data command
	buf[0] = 0x40;		// write to display, auto addr increase, normal mode
	if (obj_buf->spi.start_comm(obj_buf->spi_id, buf, 1, NULL, 0) < 0)
		return -1;
	
	// Set address command
	memset(buf, 0, sizeof(buf));
	buf[0] = 0xC0;		// address 00h
	if (obj_buf->type == TM1638_COMMON_SEGMENT) {
		for (seg = 0; seg < 10; seg++) {
			if (data_out->com_seg[seg].grid_1)		buf[1  + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_2)		buf[3  + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_3)		buf[5  + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_4)		buf[7  + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_5)		buf[9  + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_6)		buf[11 + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_7)		buf[13 + (seg / 8)] |= 1 << (seg % 8);
			if (data_out->com_seg[seg].grid_8)		buf[15 + (seg / 8)] |= 1 << (seg % 8);
		}
	}
	else {
		for (grid = 0; grid < 8; grid++) {
			if (data_out->com_grid[seg].seg_1)		buf[1  + (grid * 2)] |= 1 << (0);
			if (data_out->com_grid[seg].seg_2)		buf[1  + (grid * 2)] |= 1 << (1);
			if (data_out->com_grid[seg].seg_3)		buf[1  + (grid * 2)] |= 1 << (2);
			if (data_out->com_grid[seg].seg_4)		buf[1  + (grid * 2)] |= 1 << (3);
			if (data_out->com_grid[seg].seg_5)		buf[1  + (grid * 2)] |= 1 << (4);
			if (data_out->com_grid[seg].seg_6)		buf[1  + (grid * 2)] |= 1 << (5);
			if (data_out->com_grid[seg].seg_7)		buf[1  + (grid * 2)] |= 1 << (6);
			if (data_out->com_grid[seg].seg_8)		buf[1  + (grid * 2)] |= 1 << (7);
			if (data_out->com_grid[seg].seg_9)		buf[2  + (grid * 2)] |= 1 << (0);
			if (data_out->com_grid[seg].seg_10)		buf[2  + (grid * 2)] |= 1 << (1);
		}
	}
	if (obj_buf->spi.start_comm(obj_buf->spi_id, buf, 17, NULL, 0) < 0)
		return -1;
	
	return 0;
}


static int tm1638_read_key(tm1638_id_t id, uint8_t *data_in)
{
	struct _tm1638_obj_buf_ *obj_buf = &obj_buf_list[id];
	
	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0 || data_in == NULL) 
		return -1;

	return 0;
}


tm1638_id_t tm1638_new(struct tm1638_class *tm1638_obj, tm1638_type_t type, uint32_t port_stb, uint32_t port_clk, uint32_t port_dio)
{
	struct _tm1638_obj_buf_ *obj_buf = NULL;
	struct spi_setting setting;
	tm1638_id_t id;

	if (tm1638_obj == NULL)
		return TM1638_ID_ERR;

	if ((id = tm1638_get_obj_buf_id()) < 0)
		return TM1638_ID_ERR;

	obj_buf = &obj_buf_list[id];

	// Create SPI object
	setting.type      = SPI_TYPE_SOFT_MASTER;
	setting.port_cs   = port_stb;
	setting.port_clk  = port_clk;
	setting.port_do   = port_dio;
	setting.port_di   = port_dio;
	setting.fullplex  = 0;
	setting.msb       = 0;
	if ((obj_buf->spi_id = spi_new(&obj_buf->spi, &setting)) == SPI_ID_ERR) {
		printf("<error tm1638_new> can't create SPI object\n");
		return TM1638_ID_ERR;
	}

	obj_buf->type = type;
	obj_buf->used = 1;

	tm1638_obj->display_on    = tm1638_display_on;
	tm1638_obj->pulse_width   = tm1638_pulse_width;
	tm1638_obj->write_display = tm1638_write_display;
	tm1638_obj->read_key      = tm1638_read_key;
	
	return id;
}


int tm1638_free(tm1638_id_t id)
{
	struct _tm1638_obj_buf_ *obj_buf = &obj_buf_list[id];

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return -1;
	
	// Delete SPI object
	spi_free(obj_buf->spi_id);

	// Clear buffer
	memset(obj_buf, 0, sizeof(struct _tm1638_obj_buf_));
	
	return 0;
}


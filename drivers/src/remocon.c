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
#include <sys/time.h>		// gettimeofday
#include "gpio.h"
#include "remocon.h"


#define OBJ_BUF_SIZE		(10)
#define RMC_DATA_SIZE		(8)
#define RMC_TM_KASEIKYO		(436)
#define BW(v)				(1 << (v))


typedef struct {
	uint32_t     		ct_bit;
	uint8_t      		ct_byte;
	uint8_t      		lead;
	uint8_t      		buf[RMC_DATA_SIZE];
} rmc_bit_t;

typedef struct {
	gpio_class_t		gpio_obj;
	rmc_bit_t			rmc_bit;
	rmc_format_t		format;
	int    				port_ir;
	rmc_callback_func 	callback;
	uint64_t 			time_now;
	uint64_t 			time_old;
	uint64_t	 		time_div;
	uint8_t 			used;
} rmc_obj_buf_t;

static rmc_obj_buf_t obj_buf_list[OBJ_BUF_SIZE];


static int rmc_get_obj_buf_id(void)
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


static rmc_obj_buf_t *rmc_get_obj_buf(int id)
{
	rmc_obj_buf_t *obj_buf = &obj_buf_list[id];

	if (id >= OBJ_BUF_SIZE || obj_buf->used == 0) 
		return NULL;

	return obj_buf;
}


static int rmc_port_to_id(int port)
{
	int id = -1;

	if (port < 0)
		return -1;

	for (id = 0; id < OBJ_BUF_SIZE; id++) {
		if (obj_buf_list[id].used == 1 && obj_buf_list[id].port_ir == port)
			break;
	}

	if (id >= OBJ_BUF_SIZE)
		return -1;
	
	return id;
}


static void rmc_clear_bits(rmc_obj_buf_t *obj_buf)
{
	memset(&obj_buf->rmc_bit, 0, sizeof(rmc_bit_t));
}


static void rmc_set_lead(rmc_obj_buf_t *obj_buf)
{
	rmc_clear_bits(obj_buf);
	obj_buf->rmc_bit.lead = 1;
}


int rmc_add_bit(rmc_obj_buf_t *obj_buf, uint8_t on)
{
	if (obj_buf->rmc_bit.lead == 0)
		rmc_set_lead(obj_buf);
	else {	
		if (on) {
			if ((obj_buf->rmc_bit.ct_bit / 8) < RMC_DATA_SIZE) {
				obj_buf->rmc_bit.buf[obj_buf->rmc_bit.ct_bit / 8] |= BW(obj_buf->rmc_bit.ct_bit % 8);
			}
		}
		obj_buf->rmc_bit.ct_bit++;
	}
	
	return 0;
}


static void rmc_event_callback_kaseikyo(rmc_obj_buf_t *obj_buf)
{
	if (obj_buf->time_div >= RMC_TM_KASEIKYO && obj_buf->time_div <= (RMC_TM_KASEIKYO * 3 - 1))
		rmc_add_bit(obj_buf, 0);
	else if (obj_buf->time_div >= (RMC_TM_KASEIKYO * 3) && obj_buf->time_div <= (RMC_TM_KASEIKYO * 6 - 1))
		rmc_add_bit(obj_buf, 1);
	else if (obj_buf->time_div >= (RMC_TM_KASEIKYO * 6) && obj_buf->time_div <= (RMC_TM_KASEIKYO * 14 - 1))
		rmc_set_lead(obj_buf);
	else
		rmc_clear_bits(obj_buf);

	obj_buf->rmc_bit.ct_byte = obj_buf->rmc_bit.ct_bit == 0 ? 0 : ((obj_buf->rmc_bit.ct_bit - 1) / 8);

	if (obj_buf->rmc_bit.lead && 
		obj_buf->rmc_bit.ct_bit % 8 == 0 &&
		obj_buf->rmc_bit.ct_byte == 5) {

		// Check error
		if (obj_buf->rmc_bit.buf[0] != 0x02 || 
			obj_buf->rmc_bit.buf[1] != 0x20 ||
			obj_buf->rmc_bit.buf[2] != 0xA0) {
			return;
		}
		// Check parity
		if ((obj_buf->rmc_bit.buf[2] ^ obj_buf->rmc_bit.buf[3] ^ obj_buf->rmc_bit.buf[4]) != obj_buf->rmc_bit.buf[5]) {
			return;
		}

		if (obj_buf->callback) {
			obj_buf->callback(&obj_buf->rmc_bit.buf[3], 2);
		}
	}	
}


static void rmc_event_callback(int port, gpio_event_t event)
{
	rmc_obj_buf_t *obj_buf = NULL;
	int id = -1;
	struct timeval timeval_now;
	int ret;

	if ((id = rmc_port_to_id(port)) < 0)
		return;
	
	obj_buf = &obj_buf_list[id];

	gettimeofday(&timeval_now, NULL);
	obj_buf->time_now = timeval_now.tv_sec*1E6 + timeval_now.tv_usec;
	obj_buf->time_div = obj_buf->time_now - obj_buf->time_old;
	obj_buf->time_old = obj_buf->time_now;

	switch (obj_buf->format) {
	case RMC_FORMAT_KASEIKYO:
		rmc_event_callback_kaseikyo(obj_buf);
		break;
	}
}


int rmc_new(rmc_format_t format, int port_ir, rmc_callback_func callback)
{
	rmc_obj_buf_t *obj_buf = NULL;
	struct timeval timeval_now;
	int rd;
	
	if (callback == NULL)
		return -1;

	if ((rd = rmc_get_obj_buf_id()) < 0)
		return -1;

	obj_buf = &obj_buf_list[rd];

	// Create GPIO object
	if (gpio_new(&obj_buf->gpio_obj) < 0) {
		printf("<error remocon> can't create GPIO object\n");
		return -1;
	}

	// Create event
	if (format == RMC_FORMAT_KASEIKYO) {
		if (obj_buf->gpio_obj.ena_event(port_ir, GPIO_EVENT_RISE_EDGE, rmc_event_callback) < 0)
			return -1;
	}
	else
		return -1;

	// Assign object
	obj_buf->format   = format;
	obj_buf->port_ir  = port_ir;
	obj_buf->callback = callback;
	obj_buf->used     = 1;

	// Init current time
	gettimeofday(&timeval_now, NULL);
	obj_buf->time_now = timeval_now.tv_sec*1E6 + timeval_now.tv_usec;
	obj_buf->time_old = obj_buf->time_now;
	obj_buf->time_div = 0;

	return rd;
}


int rmc_free(int id)
{
	rmc_obj_buf_t *obj_buf = rmc_get_obj_buf(id);

	if (obj_buf == NULL)
		return -1;

	// Delete event
	obj_buf->gpio_obj.dis_event(obj_buf->port_ir);

	// Clear buffer
	memset(obj_buf, 0, sizeof(rmc_obj_buf_t));
	
	return 0;
}



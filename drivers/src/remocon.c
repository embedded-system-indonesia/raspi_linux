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


#define RMC_DESC_MAX		(10)
#define RMC_DATA_SIZE		(8)
#define RMC_TM_KASEIKYO		(436)
#define BW(v)				(1 << (v))


struct _rmc_bit_ {
	unsigned short     		ct_bit;
	unsigned char      		ct_byte;
	unsigned char      		lead;
	unsigned char      		buf[RMC_DATA_SIZE];
};

struct _rmc_desc_buf_ {
	rmc_format_t			format;
	uint32_t 				port_ir;
	rmc_callback_func 		callback;
	struct gpio_class		gpio_obj;
	unsigned long long 		time_now;
	unsigned long long 		time_old;
	unsigned long long 		time_div;
	struct _rmc_bit_		rmc_bit;
	uint8_t 				used;
};

static struct _rmc_desc_buf_ rmc_desc_buf[RMC_DESC_MAX];


static rmc_desc_t rmc_get_desc(void)
{
	rmc_desc_t rd = RMC_DESC_ERR;

	for (rd = 0; rd < RMC_DESC_MAX; rd++) {
		if (rmc_desc_buf[rd].used == 0)
			break;
	}

	if (rd >= RMC_DESC_MAX)
		return RMC_DESC_ERR;

	return rd;
}


static rmc_desc_t rmc_port_to_desc(uint32_t port)
{
	rmc_desc_t rd = RMC_DESC_ERR;

	for (rd = 0; rd < RMC_DESC_MAX; rd++) {
		if (rmc_desc_buf[rd].used == 1 && rmc_desc_buf[rd].port_ir == port)
			break;
	}

	if (rd >= RMC_DESC_MAX)
		return RMC_DESC_ERR;
	
	return rd;
}


static void rmc_clear_bits(struct _rmc_desc_buf_ *rd_buf)
{
	memset(&rd_buf->rmc_bit, 0, sizeof(struct _rmc_bit_));
}


static void rmc_set_lead(struct _rmc_desc_buf_ *rd_buf)
{
	rmc_clear_bits(rd_buf);
	rd_buf->rmc_bit.lead = 1;
}


int rmc_add_bit(struct _rmc_desc_buf_ *rd_buf, uint8_t on)
{
	if (rd_buf->rmc_bit.lead == 0)
		rmc_set_lead(rd_buf);
	else {	
		if (on) {
			if ((rd_buf->rmc_bit.ct_bit / 8) < RMC_DATA_SIZE) {
				rd_buf->rmc_bit.buf[rd_buf->rmc_bit.ct_bit / 8] |= BW(rd_buf->rmc_bit.ct_bit % 8);
			}
		}
		rd_buf->rmc_bit.ct_bit++;
	}
	
	return 0;
}


static void rmc_event_callback_kaseikyo(struct _rmc_desc_buf_ *rd_buf)
{
	if (rd_buf->time_div >= RMC_TM_KASEIKYO && rd_buf->time_div <= (RMC_TM_KASEIKYO * 3 - 1))
		rmc_add_bit(rd_buf, 0);
	else if (rd_buf->time_div >= (RMC_TM_KASEIKYO * 3) && rd_buf->time_div <= (RMC_TM_KASEIKYO * 6 - 1))
		rmc_add_bit(rd_buf, 1);
	else if (rd_buf->time_div >= (RMC_TM_KASEIKYO * 6) && rd_buf->time_div <= (RMC_TM_KASEIKYO * 14 - 1))
		rmc_set_lead(rd_buf);
	else
		rmc_clear_bits(rd_buf);

	rd_buf->rmc_bit.ct_byte = rd_buf->rmc_bit.ct_bit == 0 ? 0 : ((rd_buf->rmc_bit.ct_bit - 1) / 8);

	if (rd_buf->rmc_bit.lead && 
		rd_buf->rmc_bit.ct_bit % 8 == 0 &&
		rd_buf->rmc_bit.ct_byte == 5) {

		// Check error
		if (rd_buf->rmc_bit.buf[0] != 0x02 || 
			rd_buf->rmc_bit.buf[1] != 0x20 ||
			rd_buf->rmc_bit.buf[2] != 0xA0) {
			return;
		}
		// Check parity
		if ((rd_buf->rmc_bit.buf[2] ^ rd_buf->rmc_bit.buf[3] ^ rd_buf->rmc_bit.buf[4]) != rd_buf->rmc_bit.buf[5]) {
			return;
		}

		if (rd_buf->callback) {
			rd_buf->callback(&rd_buf->rmc_bit.buf[3], 2);
		}
	}	
}


static void rmc_event_callback(uint32_t port, gpio_event_t event)
{
	struct _rmc_desc_buf_ *rd_buf = NULL;
	rmc_desc_t rd = RMC_DESC_ERR;
	struct timeval timeval_now;
	int ret;

	if ((rd = rmc_port_to_desc(port)) < 0)
		return;
	
	rd_buf = &rmc_desc_buf[rd];

	gettimeofday(&timeval_now, NULL);
	rd_buf->time_now = timeval_now.tv_sec*1E6 + timeval_now.tv_usec;
	rd_buf->time_div = rd_buf->time_now - rd_buf->time_old;
	rd_buf->time_old = rd_buf->time_now;

	switch (rd_buf->format) {
	case RMC_FORMAT_KASEIKYO:
		rmc_event_callback_kaseikyo(rd_buf);
		break;
	}
}


rmc_desc_t rmc_new(rmc_format_t format, uint32_t port_ir, rmc_callback_func callback)
{
	struct _rmc_desc_buf_ *rd_buf = NULL;
	struct timeval timeval_now;
	rmc_desc_t rd;
	
	if (callback == NULL)
		return RMC_DESC_ERR;

	if ((rd = rmc_get_desc()) == RMC_DESC_ERR)
		return RMC_DESC_ERR;

	rd_buf = &rmc_desc_buf[rd];

	// Create GPIO object
	if (gpio_new(&rd_buf->gpio_obj) < 0) {
		printf("<ERROR RMC> can't create GPIO object\n");
		return RMC_DESC_ERR;
	}

	// Create event
	if (format == RMC_FORMAT_KASEIKYO) {
		if (rd_buf->gpio_obj.ena_event(port_ir, GPIO_EVENT_RISE_EDGE, rmc_event_callback) < 0)
			return -1;
	}
	else
		return -1;

	// Assign object
	rd_buf->format   = format;
	rd_buf->port_ir  = port_ir;
	rd_buf->callback = callback;
	rd_buf->used     = 1;

	// Init current time
	gettimeofday(&timeval_now, NULL);
	rd_buf->time_now = timeval_now.tv_sec*1E6 + timeval_now.tv_usec;
	rd_buf->time_old = rd_buf->time_now;
	rd_buf->time_div = 0;

	return rd;
}


int rmc_free(rmc_desc_t rmc_desc)
{
	struct _rmc_desc_buf_ *rd_buf = &rmc_desc_buf[rmc_desc];

	if (rmc_desc >= RMC_DESC_MAX || rd_buf->used == 0) 
		return -1;

	// Delete event
	rd_buf->gpio_obj.dis_event(rd_buf->port_ir);

	// Clear buffer
	memset(rd_buf, 0, sizeof(struct _rmc_desc_buf_));
	
	return 0;
}



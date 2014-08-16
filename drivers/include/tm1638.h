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

#ifndef _TM1638_H_
#define _TM1638_H_

#include <stdint.h>									// uint8_t, uint32_t

typedef uint8_t tm1638_type_t;
#define TM1638_COMMON_GRID					(0)		// Common cathode
#define TM1638_COMMON_SEGMENT				(1)		// Common anode

typedef uint8_t tm1638_pulse_t;
#define TM1638_PULSE_1_16					(0)
#define TM1638_PULSE_2_16					(1)
#define TM1638_PULSE_4_16					(2)
#define TM1638_PULSE_10_16					(3)
#define TM1638_PULSE_11_16					(4)
#define TM1638_PULSE_12_16					(5)
#define TM1638_PULSE_13_16					(6)
#define TM1638_PULSE_14_16					(7)


union tm1638_format {
	struct {
		uint8_t		grid_1 : 1;
		uint8_t		grid_2 : 1;
		uint8_t		grid_3 : 1;
		uint8_t		grid_4 : 1;
		uint8_t		grid_5 : 1;
		uint8_t		grid_6 : 1;
		uint8_t		grid_7 : 1;
		uint8_t		grid_8 : 1;
	} com_seg[10];
	struct {
		uint8_t		seg_1  : 1;
		uint8_t		seg_2  : 1;
		uint8_t		seg_3  : 1;
		uint8_t		seg_4  : 1;
		uint8_t		seg_5  : 1;
		uint8_t		seg_6  : 1;
		uint8_t		seg_7  : 1;
		uint8_t		seg_8  : 1;
		uint8_t		seg_9  : 1;
		uint8_t		seg_10 : 1;
		uint8_t		unused : 6;
	} com_grid[8];
};

typedef int tm1638_desc_t;
#define TM1638_DESC_ERR						(-1)


struct tm1638_class {
	int (*display_on)    (tm1638_desc_t tm1638_desc, uint8_t on_off);
	int (*pulse_width)   (tm1638_desc_t tm1638_desc, tm1638_pulse_t pulse);
	int (*write_display) (tm1638_desc_t tm1638_desc, const union tm1638_format *data_out);
	int (*read_key)      (tm1638_desc_t tm1638_desc, uint8_t *data_in);		// 4 bytes of keys
};


// Prototype
extern tm1638_desc_t tm1638_new(struct tm1638_class *tm1638_obj, tm1638_type_t type, uint32_t port_stb, uint32_t port_clk, uint32_t port_dio);
extern int tm1638_free(tm1638_desc_t tm1638_desc);


#endif


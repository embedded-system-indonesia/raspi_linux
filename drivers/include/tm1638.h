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


typedef union {
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
} tm1638_format_t;


typedef struct {
	int (*display_on)    (int id, uint8_t on_off);
	int (*pulse_width)   (int id, tm1638_pulse_t pulse);
	int (*write_display) (int id, const tm1638_format_t *data_out);
	int (*read_key)      (int id, uint8_t *data_in);		// 4 bytes of keys
} tm1638_class_t;


// Prototype
extern int tm1638_new(tm1638_class_t *tm1638_obj, tm1638_type_t type, int port_stb, int port_clk, int port_dio);
extern int tm1638_free(int id);


#endif


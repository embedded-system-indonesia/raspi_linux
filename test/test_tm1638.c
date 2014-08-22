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
#include <unistd.h>			// usleep, sleep
#include "tm1638.h"


int main()
{
	tm1638_class_t tm1638;
	tm1638_format_t format;
	int id;
	uint8_t ct = 10;

	if ((id = tm1638_new(&tm1638, TM1638_COMMON_SEGMENT, 27, 4, 17)) < 0)
		return -1;

	tm1638.pulse_width(id, TM1638_PULSE_14_16);

	// Assume that we have LED 4 digit 7 segment with common segment
	memset(&format, 0, sizeof(format));
	
	// Digit 1: Char "1"
	format.com_seg[0].grid_1 = 0;
	format.com_seg[0].grid_2 = 1;
	format.com_seg[0].grid_3 = 1;
	format.com_seg[0].grid_4 = 0;
	format.com_seg[0].grid_5 = 0;
	format.com_seg[0].grid_6 = 0;
	format.com_seg[0].grid_7 = 0;
	format.com_seg[0].grid_8 = 0;		// dot

	// Digit 2: Char "2"
	format.com_seg[1].grid_1 = 1;
	format.com_seg[1].grid_2 = 1;
	format.com_seg[1].grid_3 = 0;
	format.com_seg[1].grid_4 = 1;
	format.com_seg[1].grid_5 = 1;
	format.com_seg[1].grid_6 = 0;
	format.com_seg[1].grid_7 = 1;

	// Digit 3: Char "3"
	format.com_seg[2].grid_1 = 1;
	format.com_seg[2].grid_2 = 1;
	format.com_seg[2].grid_3 = 1;
	format.com_seg[2].grid_4 = 1;
	format.com_seg[2].grid_5 = 0;
	format.com_seg[2].grid_6 = 0;
	format.com_seg[2].grid_7 = 1;

	// Digit 4: Char "4"
	format.com_seg[3].grid_1 = 0;
	format.com_seg[3].grid_2 = 1;
	format.com_seg[3].grid_3 = 1;
	format.com_seg[3].grid_4 = 0;
	format.com_seg[3].grid_5 = 0;
	format.com_seg[3].grid_6 = 1;
	format.com_seg[3].grid_7 = 1;
	
	tm1638.write_display(id, &format);

	while (ct) {
		tm1638.display_on(id, 1);
		sleep(1);
		tm1638.display_on(id, 0);
		sleep(1);
		ct--;
	}
	
	tm1638_free(id);
	
	return 0;
}


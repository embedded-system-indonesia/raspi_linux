/* 
 * Code to test IO expander PCF8574
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

#include <unistd.h>				// usleep, sleep
#include "ioexp_pcf8574.h"


int main()
{
	struct ioexp_pcf8574_class ioexp_obj;
	ioexp_pcf8574_desc_t desc;

	if ((desc = ioexp_pcf8574_new(&ioexp_obj, 23, 24)) == IOEXP_PCF8574_DESC_ERR)
		return -1;

	ioexp_obj.set_all_port(desc, 0xFF);
	sleep(1);
	
	// Loop forever, can add timeout if necessary
	while (1) {
		ioexp_obj.set_one_port(desc, 2, 1);
		ioexp_obj.set_one_port(desc, 3, 0);
		sleep(1);
		ioexp_obj.set_one_port(desc, 2, 0);
		ioexp_obj.set_one_port(desc, 3, 1);
		sleep(1);
	}
	
	ioexp_pcf8574_free(desc);
	
	return 0;
}


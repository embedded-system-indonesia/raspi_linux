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
	iopcf_class_t ioexp_obj;
	int id;

	if ((id = ioexp_pcf8574_new(&ioexp_obj, 23, 24)) < 0)
		return -1;

	ioexp_obj.set_all_port(id, 0xFF);
	sleep(1);
	
	// Loop forever, can add timeout if necessary
	while (1) {
		ioexp_obj.set_one_port(id, 2, 1);
		ioexp_obj.set_one_port(id, 3, 0);
		sleep(1);
		ioexp_obj.set_one_port(id, 2, 0);
		ioexp_obj.set_one_port(id, 3, 1);
		sleep(1);
	}
	
	ioexp_pcf8574_free(id);
	
	return 0;
}



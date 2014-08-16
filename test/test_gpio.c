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


#include "gpio.h"


int main()
{
	uint8_t ct = 10;
	uint8_t led1, led2;
	struct gpio_class gpio;

	gpio_new(&gpio);

	led1 = 10;
	led2 = 9;
	gpio.set_mode(led1, GPIO_MODE_OUTPUT);
	gpio.set_mode(led2, GPIO_MODE_INPUT);
	
	while (ct) {
		gpio.set_level(led1, 1);
		gpio.set_pull(led2, GPIO_PULL_DOWN);
		sleep(1);
		
		gpio.set_level(led1, 0);
		gpio.set_pull(led2, GPIO_PULL_UP);
		sleep(1);
		ct --;
	}

	gpio_free();
	
	return 0;
}


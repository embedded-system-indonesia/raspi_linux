/* 
 * Code to test I2C communication
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

#include <unistd.h>			// usleep, sleep
#include "i2c.h"


/* I test use IO expander PCF8574 */
#define I2C_SLAVE_ADDR		(0x27)


static void make_data(uint8_t *data_out, uint32_t *len_out, uint8_t port)
{
	data_out[0] = I2C_SLAVE_ADDR << 1;
	data_out[1] = port;
	*len_out    = 2;
}


int main()
{
	struct i2c_class i2c_obj;
	i2c_desc_t desc;
	uint8_t    data_out[2];
	uint32_t   len_out;
	uint16_t   ct = 0;
	uint16_t   port = 0;

	if ((desc = i2c_new(&i2c_obj, I2C_TYPE_SOFT_MASTER, I2C_SPEED_FULL, 23, 24)) == I2C_DESC_ERR)
		return -1;

	// Loop forever, can add timeout if necessary
	while (1) {
		port = (ct << 6) | (ct << 4) | (ct << 2) | ct;
		make_data(data_out, &len_out, port);
		i2c_obj.start_comm(desc, data_out, len_out, NULL, 0);

		ct++;
		if (ct >= 4)
			ct = 0;

		sleep(1);
	}
	
	i2c_free(desc);
	
	return 0;
}



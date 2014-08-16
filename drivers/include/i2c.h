/* 
 * I2C communication controller
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

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>							// uint8_t, uint32_t


// I2C descriptor
typedef int i2c_desc_t;
#define I2C_DESC_ERR						(-1)

// I2C type
typedef uint8_t i2c_type_t;
#define I2C_TYPE_SOFT_MASTER				(0)

// I2C speed
typedef uint8_t i2c_speed_t;
#define I2C_SPEED_HIGH						(0)
#define I2C_SPEED_FULL						(1)

// I2C port
#define I2C_PORT_UNUSED						(0xFFFF)


struct i2c_class {
	int (*start_comm) (i2c_desc_t i2c_desc, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in);
};


// Prototype
extern i2c_desc_t i2c_new(struct i2c_class *i2c_obj, i2c_type_t type, i2c_speed_t speed, uint32_t port_scl, uint32_t port_sda);
extern int i2c_free(i2c_desc_t i2c_desc);


#endif


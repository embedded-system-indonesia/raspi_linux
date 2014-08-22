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


/*
 * Parameter "int port_xx"
 *       port_xx < 0  : unused port
 *       port_xx >= 0 : valid port
 *
 * Parameter "int id"
 *       id < 0  : invalid ID
 *       id >= 0 : valid ID
 *       *) id will be used to select device
 *
 * Return function "int"
 *       ret < 0 : error
 *       ret = 0 : OK
 *       *) special for function "i2c_new", it will return ID
 */


// I2C type
typedef uint8_t i2c_type_t;
#define I2C_TYPE_SOFT_MASTER				(0)

// I2C speed
typedef uint8_t i2c_speed_t;
#define I2C_SPEED_HIGH						(0)
#define I2C_SPEED_FULL						(1)


typedef struct {
	int (*start_comm) (int id, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in);
} i2c_class_t;


// Prototype
extern int i2c_new(i2c_class_t *i2c_obj, i2c_type_t type, i2c_speed_t speed, int port_scl, int port_sda);
extern int i2c_free(int id);


#endif


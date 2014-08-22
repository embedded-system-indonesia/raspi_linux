/* 
 * IO Expander PCF8574
 * 8 bit IO contolled using I2C
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

#ifndef _IOEXP_PCF8574_H_
#define _IOEXP_PCF8574_H_

#include <stdint.h>							// uint8_t, uint32_t


typedef struct {
	int      (*set_all_port) (int id, int ports);
	uint32_t (*get_all_port) (int id);
	int      (*set_one_port) (int id, int port, uint8_t level);
	uint8_t  (*get_one_port) (int id, int port);	
} iopcf_class_t;


// Prototype
extern int ioexp_pcf8574_new(iopcf_class_t *ioexp_obj, int port_scl, int port_sda);
extern int ioexp_pcf8574_free(int id);


#endif


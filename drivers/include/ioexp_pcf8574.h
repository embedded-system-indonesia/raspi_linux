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


// IO expander descriptor
typedef int ioexp_pcf8574_desc_t;
#define IOEXP_PCF8574_DESC_ERR				(-1)

// IO expander port
#define IOEXP_PCF8574_PORT_UNUSED			(0xFFFF)


struct ioexp_pcf8574_class {
	int      (*set_all_port) (ioexp_pcf8574_desc_t desc, uint32_t ports);
	uint32_t (*get_all_port) (ioexp_pcf8574_desc_t desc);
	int      (*set_one_port) (ioexp_pcf8574_desc_t desc, uint32_t port, uint8_t level);
	uint8_t  (*get_one_port) (ioexp_pcf8574_desc_t desc, uint32_t port);	
};


// Prototype
extern ioexp_pcf8574_desc_t ioexp_pcf8574_new(struct ioexp_pcf8574_class *ioexp_obj, uint32_t port_scl, uint32_t port_sda);
extern int ioexp_pcf8574_free(ioexp_pcf8574_desc_t desc);


#endif


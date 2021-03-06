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

#ifndef _REMOCON_H_
#define _REMOCON_H_

#include <stdint.h>									// uint8_t, uint32_t


// Remocon format
typedef uint8_t rmc_format_t;
#define RMC_FORMAT_KASEIKYO					(0)

// Callback function
typedef void (*rmc_callback_func)(uint8_t *data_rcv, int len);


// Prototype
extern int rmc_new(rmc_format_t format, int port_ir, rmc_callback_func callback); // return id
extern int rmc_free(int id);


#endif


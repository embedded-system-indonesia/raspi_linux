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

#ifndef _SPI_H_
#define _SPI_H_

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
 *       *) special for function "spi_new", it will return ID
 */


// SPI type
typedef int spi_type_t;
#define SPI_TYPE_SOFT_MASTER				(0)

typedef struct {
	spi_type_t type;
	int        port_cs;
	int        port_clk;
	int        port_do;
	int        port_di;
	uint8_t    msb;
	uint8_t    fullplex;
} spi_setting_t;

typedef struct {
	int (*start_comm) (int id, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in);
} spi_class_t;


// Prototype
extern int spi_new(spi_class_t *spi_obj, const spi_setting_t *setting);
extern int spi_free(int id);

#endif


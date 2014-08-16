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

// SPI descriptor
typedef int spi_desc_t;
#define SPI_DESC_ERR						(-1)

// SPI type
typedef int spi_type_t;
#define SPI_TYPE_SOFT_MASTER				(0)

#define SPI_PORT_UNUSED						(0xFFFF)


struct spi_setting {
	spi_type_t type;
	uint32_t   port_cs;
	uint32_t   port_clk;
	uint32_t   port_do;
	uint32_t   port_di;
	uint8_t    msb;
	uint8_t    fullplex;
};

struct spi_class {
	int (*start_comm) (spi_desc_t spi_desc, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in);
};


// Prototype
extern spi_desc_t spi_new(struct spi_class *spi_obj, const struct spi_setting *setting);
extern int spi_free(spi_desc_t spi_desc);

#endif


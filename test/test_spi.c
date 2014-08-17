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


#include <string.h>			// memset, memcpy
#include <unistd.h>			// usleep, sleep
#include "spi.h"


int main()
{
	struct spi_class   spi;
	struct spi_setting setting;
	spi_id_t           spi_id;
	uint8_t            data_out[20], data_in[20];
	uint32_t           len_out, len_in;
	uint8_t            ct = 10;

	setting.type      = SPI_TYPE_SOFT_MASTER;
	setting.port_cs   = 27;
	setting.port_clk  = 4;
	setting.port_do   = 17;
	setting.port_di   = 17;
	setting.fullplex  = 0;
	setting.msb       = 0;

	spi_id = spi_new(&spi, &setting);

	if (spi_id != SPI_ID_ERR) {
		data_out[0] = 0x40;
		len_out     = 1;
		spi.start_comm(spi_id, data_out, len_out, NULL, 0);

		data_out[0] = 0xC0;
		len_out     = 17;
		memset(&data_out[1], 0xFF, 16);
		spi.start_comm(spi_id, data_out, len_out, NULL, 0);

		data_out[0] = 0x8F;
		len_out     = 1;
		spi.start_comm(spi_id, data_out, len_out, NULL, 0);

		while (ct) {
			sleep(1);
			ct --;
		}

		data_out[0] = 0xC0;
		len_out     = 17;
		memset(&data_out[1], 0, 16);
		spi.start_comm(spi_id, data_out, len_out, NULL, 0);
	}
	
	spi_free(spi_id);
	
	return 0;
}



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


#include <unistd.h>			// usleep
#include <string.h>			// memset, memcpy
#include <stdio.h>			// printf, snprintf
#include "gpio.h"
#include "spi.h"


#define SPI_DESC_MAX		(10)
#define SPI_BIT_DELAY		(2)  // 2 usec
#define BW(v)				(1 << (v))


struct _spi_desc_buf_ {
	struct spi_setting	setting;
	uint8_t 		   	used;
};

struct _spi_info_ {
	struct gpio_class	gpio;
	uint32_t          	gpio_max;
};


static struct _spi_desc_buf_ spi_desc_buf[SPI_DESC_MAX];
static struct _spi_info_ spi_info;


static spi_desc_t spi_get_desc(void)
{
	spi_desc_t sd = SPI_DESC_ERR;

	for (sd = 0; sd < SPI_DESC_MAX; sd++) {
		if (spi_desc_buf[sd].used == 0)
			break;
	}

	if (sd >= SPI_DESC_MAX)
		return SPI_DESC_ERR;

	return sd;
}


static int spi_setup_port(spi_desc_t spi_desc)
{
	uint32_t port_cs  = spi_desc_buf[spi_desc].setting.port_cs;
	uint32_t port_clk = spi_desc_buf[spi_desc].setting.port_clk;
	uint32_t port_do  = spi_desc_buf[spi_desc].setting.port_do;
	uint32_t port_di  = spi_desc_buf[spi_desc].setting.port_di;
	spi_type_t type   = spi_desc_buf[spi_desc].setting.type;
	int ret = -1;
	
	switch (type) {
	case SPI_TYPE_SOFT_MASTER:
		ret = 0;
		if (port_cs < spi_info.gpio_max) {
			spi_info.gpio.set_mode(port_cs, GPIO_MODE_OUTPUT);
			spi_info.gpio.set_level(port_cs, 1);
		}
		if (port_clk < spi_info.gpio_max) {
			spi_info.gpio.set_mode(port_clk, GPIO_MODE_OUTPUT);
			spi_info.gpio.set_level(port_clk, 1);
		}
		if (port_do < spi_info.gpio_max) {
			spi_info.gpio.set_mode(port_do, GPIO_MODE_OUTPUT);
			spi_info.gpio.set_level(port_do, 1);
		}
		if (port_di < spi_info.gpio_max)
			spi_info.gpio.set_mode(port_do, GPIO_MODE_INPUT);
		break;
	}

	return ret;
}


static void spi_port_set_lvl(uint32_t port, uint8_t level)
{
	if (port < spi_info.gpio_max)
		spi_info.gpio.set_level(port, level);
}


static uint8_t spi_port_get_lvl(uint32_t port)
{
	if (port >= spi_info.gpio_max)
		return 0;
	
	return spi_info.gpio.get_level(port);
}


static void spi_port_set_mode(uint32_t port, gpio_mode_t mode)
{
	if (port < spi_info.gpio_max)
		spi_info.gpio.set_mode(port, mode);
}


static int spi_start_comm_soft_master(spi_desc_t spi_desc, unsigned char *data_out, uint32_t len_out, unsigned char *data_in, uint32_t len_in)
{
	uint32_t port_cs  = spi_desc_buf[spi_desc].setting.port_cs;
	uint32_t port_clk = spi_desc_buf[spi_desc].setting.port_clk;
	uint32_t port_do  = spi_desc_buf[spi_desc].setting.port_do;
	uint32_t port_di  = spi_desc_buf[spi_desc].setting.port_di;
	uint8_t  msb      = spi_desc_buf[spi_desc].setting.msb;
	uint8_t  fullplex = spi_desc_buf[spi_desc].setting.fullplex;
	uint32_t ctdat;
	uint8_t  ctbit;
	
	spi_port_set_lvl(port_cs, 1);
	spi_port_set_lvl(port_clk, 1);
	spi_port_set_lvl(port_do, 1);
	usleep(SPI_BIT_DELAY);

	spi_port_set_lvl(port_cs, 0);
	usleep(SPI_BIT_DELAY);

	spi_port_set_lvl(port_clk, 0);

	for (ctdat = 0; ctdat < len_out; ctdat++) {
		for (ctbit = 0; ctbit < 8; ctbit++) {
			spi_port_set_mode(port_do, GPIO_MODE_OUTPUT);
			if (msb) {
				if (data_out[ctdat] & BW(7 - ctbit)) {
					spi_port_set_lvl(port_do, 1);
				}
				else {
					spi_port_set_lvl(port_do, 0);
				}
			}
			else {
				if (data_out[ctdat] & BW(ctbit)) {
					spi_port_set_lvl(port_do, 1);
				}
				else {
					spi_port_set_lvl(port_do, 0);
				}
			}

			usleep(SPI_BIT_DELAY);
			spi_port_set_lvl(port_clk, 1);
			usleep(SPI_BIT_DELAY);
			spi_port_set_lvl(port_clk, 0);

			// Read data
			if (fullplex) {
				// Ful duplex
				if (data_in && ctdat < len_in) {
					spi_port_set_mode(port_di, GPIO_MODE_INPUT);
					if (msb) {
						if (spi_port_get_lvl(port_di)) {
							data_in[ctdat] |= BW(7 - ctbit);
						}
						else {
							data_in[ctdat] &= ~(BW(7 - ctbit));
						}
					}
					else {
						if (spi_port_get_lvl(port_di)) {
							data_in[ctdat] |= BW(ctbit);
						}
						else {
							data_in[ctdat] &= ~(BW(ctbit));
						}
					}
				}
			}
		}
	}

	if (!fullplex && data_in) {
		// Half plex
		spi_port_set_lvl(port_di, GPIO_MODE_INPUT);

		for (ctdat = 0; ctdat < len_in; ctdat++) {
			for (ctbit = 0; ctbit < 8; ctbit++) {

				usleep(SPI_BIT_DELAY);
				spi_port_set_lvl(port_clk, 1);
				usleep(SPI_BIT_DELAY);
				
				if (data_in && ctdat < len_in) {
					if (msb) {
						if (spi_port_get_lvl(port_di)) {
							data_in[ctdat] |= BW(7 - ctbit);
						}
						else {
							data_in[ctdat] &= ~(BW(7 - ctbit));
						}
					}
					else {
						if (spi_port_get_lvl(port_di)) {
							data_in[ctdat] |= BW(ctbit);
						}
						else {
							data_in[ctdat] &= ~(BW(ctbit));
						}
					}
				}

				spi_port_set_lvl(port_clk, 0);
			}		
		}	

	}

	usleep(SPI_BIT_DELAY);
	spi_port_set_lvl(port_do, GPIO_MODE_OUTPUT);
	spi_port_set_lvl(port_clk, 1);
	spi_port_set_lvl(port_do, 1);
	usleep(SPI_BIT_DELAY);
	spi_port_set_lvl(port_cs, 1);	

	return 0;
}


static int spi_start_comm(spi_desc_t spi_desc, unsigned char *data_out, uint32_t len_out, unsigned char *data_in, uint32_t len_in)
{
	uint8_t ret = -1;
	
	if (spi_desc >= SPI_DESC_MAX || spi_desc_buf[spi_desc].used == 0) 
		return -1;

	switch (spi_desc_buf[spi_desc].setting.type) {
	case SPI_TYPE_SOFT_MASTER:
		ret = spi_start_comm_soft_master(spi_desc, data_out, len_out, data_in, len_in);
		break;
	}
	
	return ret;
}


spi_desc_t spi_new(struct spi_class *spi_obj, const struct spi_setting *setting)
{
	spi_desc_t sd;
	
	if (spi_obj == NULL || setting == NULL)
		return SPI_DESC_ERR;

	if ((sd = spi_get_desc()) == SPI_DESC_ERR)
		return SPI_DESC_ERR;

	// Store data
	memcpy(&spi_desc_buf[sd].setting, setting, sizeof(struct spi_setting));
	spi_desc_buf[sd].used = 1;

	// Create object
	spi_obj->start_comm = spi_start_comm;

	// Load GPIO
	gpio_new(&spi_info.gpio);
	spi_info.gpio_max = spi_info.gpio.get_port_num();

	// Setup port
	if (spi_setup_port(sd) < 0)
		return SPI_DESC_ERR;

	return sd;
}


int spi_free(spi_desc_t spi_desc)
{
	if (spi_desc >= SPI_DESC_MAX || spi_desc_buf[spi_desc].used == 0) 
		return -1;

	memset(&spi_desc_buf[spi_desc], 0, sizeof(struct _spi_desc_buf_));
	
	return 0;
}



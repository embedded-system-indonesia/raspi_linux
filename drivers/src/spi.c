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


#define OBJ_BUF_SIZE			(10)
#define SPI_BIT_DELAY			(2)  // 2 usec
#define BW(v)					(1 << (v))


struct _spi_obj_buf_ {
	struct spi_setting	setting;
	uint8_t 		   	used;
};

struct _spi_info_ {
	struct gpio_class	gpio;
};


static struct _spi_obj_buf_ obj_buf_list[OBJ_BUF_SIZE];
static struct _spi_info_ spi_info;


static int spi_get_obj_buf_id(void)
{
	int id = -1;

	for (id = 0; id < OBJ_BUF_SIZE; id++) {
		if (obj_buf_list[id].used == 0)
			break;
	}

	if (id >= OBJ_BUF_SIZE)
		return -1;

	return id;
}


static int spi_setup_port(spi_id_t spi_id)
{
	uint32_t port_cs  = obj_buf_list[spi_id].setting.port_cs;
	uint32_t port_clk = obj_buf_list[spi_id].setting.port_clk;
	uint32_t port_do  = obj_buf_list[spi_id].setting.port_do;
	uint32_t port_di  = obj_buf_list[spi_id].setting.port_di;
	spi_type_t type   = obj_buf_list[spi_id].setting.type;
	int ret = -1;
	
	switch (type) {
	case SPI_TYPE_SOFT_MASTER:
		ret = 0;
		spi_info.gpio.set_mode(port_cs, GPIO_MODE_OUTPUT);
		spi_info.gpio.set_level(port_cs, 1);
		spi_info.gpio.set_mode(port_clk, GPIO_MODE_OUTPUT);
		spi_info.gpio.set_level(port_clk, 1);
		spi_info.gpio.set_mode(port_do, GPIO_MODE_OUTPUT);
		spi_info.gpio.set_level(port_do, 1);
		spi_info.gpio.set_mode(port_do, GPIO_MODE_INPUT);
		break;
	}

	return ret;
}


static void spi_port_set_lvl(uint32_t port, uint8_t level)
{
	spi_info.gpio.set_level(port, level);
}


static uint8_t spi_port_get_lvl(uint32_t port)
{
	return spi_info.gpio.get_level(port);
}


static void spi_port_set_mode(uint32_t port, gpio_mode_t mode)
{
	spi_info.gpio.set_mode(port, mode);
}


static int spi_start_comm_soft_master(spi_id_t spi_id, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in)
{
	uint32_t port_cs  = obj_buf_list[spi_id].setting.port_cs;
	uint32_t port_clk = obj_buf_list[spi_id].setting.port_clk;
	uint32_t port_do  = obj_buf_list[spi_id].setting.port_do;
	uint32_t port_di  = obj_buf_list[spi_id].setting.port_di;
	uint8_t  msb      = obj_buf_list[spi_id].setting.msb;
	uint8_t  fullplex = obj_buf_list[spi_id].setting.fullplex;
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


static int spi_start_comm(spi_id_t spi_id, uint8_t *data_out, uint32_t len_out, uint8_t *data_in, uint32_t len_in)
{
	uint8_t ret = -1;
	
	if (spi_id >= OBJ_BUF_SIZE || obj_buf_list[spi_id].used == 0) 
		return -1;

	switch (obj_buf_list[spi_id].setting.type) {
	case SPI_TYPE_SOFT_MASTER:
		ret = spi_start_comm_soft_master(spi_id, data_out, len_out, data_in, len_in);
		break;
	}
	
	return ret;
}


spi_id_t spi_new(struct spi_class *spi_obj, const struct spi_setting *setting)
{
	spi_id_t id;
	
	if (spi_obj == NULL || setting == NULL)
		return SPI_ID_ERR;

	if ((id = spi_get_obj_buf_id()) < 0)
		return SPI_ID_ERR;

	// Store data
	memcpy(&obj_buf_list[id].setting, setting, sizeof(struct spi_setting));
	obj_buf_list[id].used = 1;

	// Create object
	spi_obj->start_comm = spi_start_comm;

	// Load GPIO
	gpio_new(&spi_info.gpio);

	// Setup port
	if (spi_setup_port(id) < 0)
		return SPI_ID_ERR;

	return id;
}


int spi_free(spi_id_t spi_id)
{
	if (spi_id >= OBJ_BUF_SIZE || obj_buf_list[spi_id].used == 0) 
		return -1;

	memset(&obj_buf_list[spi_id], 0, sizeof(struct _spi_obj_buf_));
	
	return 0;
}



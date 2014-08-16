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

#include <sys/mman.h>		// mmap
#include <fcntl.h>			// open, close
#include <unistd.h>			// write
#include <stdio.h>			// printf, snprintf
#include <sys/epoll.h>		// epoll_wait
#include <pthread.h>		// pthread_create, pthread_exit, 
#include <string.h>			// memset
#include "gpio.h"

// Peripheral address
#define BCM_BASE_ADDR					(0x20000000)
#define GPIO_BASE_ADDR					(BCM_BASE_ADDR + 0x200000)

// GPIO register offset
#define GPIO_OFFSET_FSEL				(0)
#define GPIO_OFFSET_OUTPUT_SET			(7)
#define GPIO_OFFSET_OUTPUT_CLEAR		(10)
#define GPIO_OFFSET_LEVEL				(13)
#define GPIO_OFFSET_EVENT_DET_STAT		(16)
#define GPIO_OFFSET_RISE_DET_ENA		(19)
#define GPIO_OFFSET_FALL_DET_ENA		(22)
#define GPIO_OFFSET_HIGH_DET_ENA		(25)
#define GPIO_OFFSET_LOW_DET_ENA			(28)
#define GPIO_OFFSET_ASYNC_RISE_DET		(31)
#define GPIO_OFFSET_ASYNC_FALL_DET		(34)
#define GPIO_OFFSET_PULL_ENA			(37)
#define GPIO_OFFSET_PULL_CLK_ENA		(38)

// GPIO description
#define GPIO_PORT_MAX					(54)

// GPIO function select
#define GPFSEL_INPUT					(0)
#define GPFSEL_OUTPUT					(1)
#define GPFSEL_FUNC_0					(4)
#define GPFSEL_FUNC_1					(5)
#define GPFSEL_FUNC_2					(6)
#define GPFSEL_FUNC_3					(7)
#define GPFSEL_FUNC_4					(3)
#define GPFSEL_FUNC_5					(2)
#define GPFSEL_NUM						(8)

// GPIO Pull Up / Down
#define GPPUD_PULL_OFF					(0)
#define GPPUD_PULL_DOWN					(1)
#define GPPUD_PULL_UP					(2)

// memory description
#define BLOCK_SIZE						(4 * 1024)


struct _gpio_setting_ {
	gpio_mode_t   		mode;
	uint8_t		 		level;
	gpio_pull_t   		pull;
	gpio_event_t  		event;
	gpio_ev_callback 	evclback;
	int           		fd;
};

struct _gpio_info_ {
	int 				epoll;
	uint8_t				thread_on;
	int           		fd_map;
	volatile uint32_t 	*map_gpio;
	uint8_t				init;
};


// Prototype descriptions
static struct _gpio_setting_ gpio_setting[GPIO_PORT_MAX];
static struct _gpio_info_    gpio_info;


const uint8_t TBL_GPIO_FSEL_OFFSET[]  = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// gpio 0-9
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// gpio 10-19
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// gpio 20-29
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// gpio 30-39
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4,		// gpio 40-49
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,		// gpio 50-59
};

const uint8_t TBL_GPIO_FSEL_SHIFT[]  = {
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
};


static int gpio_dis_event(uint32_t port);


static int gpio_file_write_str(const char *file_name, const char *arg, int len)
{
    int fd;

    if ((fd = open(file_name, O_WRONLY)) < 0)
       return -1;

    write(fd, arg, len);
    close(fd);

    return 0;
}


static int gpio_file_write_num(const char *file_name, uint32_t numeric)
{
    int  fd;
	int  len;
	char str[11];

    if ((fd = open(file_name, O_WRONLY)) < 0)
       return -1;

    len = snprintf(str, sizeof(str), "%ld", numeric);
    write(fd, str, len);
    close(fd);

    return 0;
}


static void gpio_delay_cyc(uint32_t cyc)
{
	uint32_t ct;
	
	for (ct = 0; ct < cyc; ct++) {
		asm volatile("nop");
	}
}


static int gpio_set_edge(uint32_t port, gpio_event_t event)
{
	char str_file[256];
	char str_arg[10];
	int  str_len = 0;

	snprintf(str_file, sizeof(str_file), "/sys/class/gpio/gpio%ld/edge", port);
	
	if ((event & GPIO_EVENT_RISE_EDGE) && (event & GPIO_EVENT_FALL_EDGE))
		str_len = snprintf(str_arg, sizeof(str_arg), "both");
	else if (event & GPIO_EVENT_RISE_EDGE)
		str_len = snprintf(str_arg, sizeof(str_arg), "rising");
	else if (event & GPIO_EVENT_FALL_EDGE)
		str_len = snprintf(str_arg, sizeof(str_arg), "falling");

	if (str_len == 0)
		return -1;
	
	if (gpio_file_write_str(str_file, str_arg, str_len) < 0)
		return -1;
		
	return 0;
}


static void *gpio_thread_event(void *arg)
{
    struct epoll_event ep_event;
    int n;
	uint32_t port;
    char buf;
	uint8_t ev_init = 0;

	gpio_info.thread_on = 1;

	while (gpio_info.thread_on) {

        if ((n = epoll_wait(gpio_info.epoll, &ep_event, 1, -1)) == -1)
			break;

        if (n == 0)
			continue;
		
		if (ev_init == 0) {
			ev_init = 1;
			continue;
		}

        lseek(ep_event.data.fd, 0, SEEK_SET);
        if (read(ep_event.data.fd, &buf, 1) != 1)
			break;

		for (port = 0; port < GPIO_PORT_MAX; port++) {
			if (gpio_setting[port].fd == ep_event.data.fd)
				break;
		}
		if (port < GPIO_PORT_MAX) {
			if (gpio_setting[port].evclback)
				gpio_setting[port].evclback(port, (buf & 1) ? GPIO_EVENT_RISE_EDGE : GPIO_EVENT_FALL_EDGE);
		}
	}
	
	gpio_info.thread_on = 0;
    pthread_exit(NULL);
}


static int gpio_set_mode(uint32_t port, gpio_mode_t mode)
{
	uint8_t offset, shift, mode_sel;
	
	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return -1;

	offset = TBL_GPIO_FSEL_OFFSET[port];
	shift  = TBL_GPIO_FSEL_SHIFT[port];

	if (mode == GPIO_MODE_OUTPUT)
		mode_sel = GPFSEL_OUTPUT;
	else
		mode_sel = GPFSEL_INPUT;

	*(gpio_info.map_gpio + offset) &= ~(7 << shift);
	*(gpio_info.map_gpio + offset) |= (mode_sel & 7) << shift;
	
	gpio_setting[port].mode = mode;
		
	return 0;
}


static gpio_mode_t gpio_get_mode(uint32_t port)
{
	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return GPIO_MODE_INPUT;

	return gpio_setting[port].mode;
}


static int gpio_set_level(uint32_t port, uint8_t level)
{
	uint8_t offset;
	
	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return -1;

	if (level)
		offset = GPIO_OFFSET_OUTPUT_SET + (port / 32);
	else
		offset = GPIO_OFFSET_OUTPUT_CLEAR + (port / 32);

	*(gpio_info.map_gpio + offset) = 1 << (port % 32);

	gpio_setting[port].level = level;

	return 0;
}


static uint8_t gpio_get_level(uint32_t port)
{
	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return 0;

	return gpio_setting[port].level;
}


int gpio_set_pull(uint32_t port, gpio_pull_t pull)
{
	uint8_t offset_ena, offset_clk, pullval;

	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return -1;

	offset_ena = GPIO_OFFSET_PULL_ENA;
	offset_clk = GPIO_OFFSET_PULL_CLK_ENA + (port / 32);

	if (pull == GPIO_PULL_UP)
		pullval = GPPUD_PULL_UP;
	else if (pull == GPIO_PULL_DOWN)
		pullval = GPPUD_PULL_DOWN;
	else
		pullval = GPIO_PULL_NONE;
	
	*(gpio_info.map_gpio + offset_ena) &= ~3;
	*(gpio_info.map_gpio + offset_ena) |= pullval;
	gpio_delay_cyc(150);
	
	*(gpio_info.map_gpio + offset_clk) = 1 << (port % 32);
	gpio_delay_cyc(150);

	*(gpio_info.map_gpio + offset_ena) &= ~3;
	*(gpio_info.map_gpio + offset_clk) = 0;

	gpio_setting[port].pull = pull;
	
	return 0;
}


static gpio_pull_t gpio_get_pull(uint32_t port)
{
	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return GPIO_PULL_NONE;

	return gpio_setting[port].pull;
}


static int gpio_ena_event(uint32_t port, gpio_event_t event, gpio_ev_callback evclback)
{
    struct epoll_event ep_event;
    pthread_t thread_id;
	char str_file[256];

	if (port >= GPIO_PORT_MAX || gpio_info.init == 0  || evclback == NULL)
		return -1;

	// Set input
	if (gpio_set_mode(port, GPIO_MODE_INPUT) < 0)
		return -1;

	// Export GPIO
	snprintf(str_file, sizeof(str_file), "/sys/class/gpio/export");
	if (gpio_file_write_num(str_file, port) < 0)
		return;
	
	// Set edge
	if (gpio_set_edge(port, event) < 0)
		return -1;

	// Get file descriptor of GPIO value
	gpio_setting[port].fd = -1;
	snprintf(str_file, sizeof(str_file), "/sys/class/gpio/gpio%ld/value", port);
    if ((gpio_setting[port].fd = open(str_file, O_RDONLY | O_NONBLOCK)) < 0) {
       return -1;
   	}

	// Create epoll event
	if ((gpio_info.epoll = epoll_create(1)) == -1)
		return -1;

	// Store data
	gpio_setting[port].event    = event;
	gpio_setting[port].evclback = evclback;

	// Add epoll event
    ep_event.events  = EPOLLIN | EPOLLET | EPOLLPRI;
	ep_event.data.fd = gpio_setting[port].fd;
    if (epoll_ctl(gpio_info.epoll, EPOLL_CTL_ADD, gpio_setting[port].fd, &ep_event) == -1) {
        gpio_dis_event(port);
        return -1;
    }

	// Create thread
	if (!gpio_info.thread_on) {
        if (pthread_create(&thread_id, NULL, gpio_thread_event, NULL) != 0) {
			gpio_dis_event(port);
			return -1;
        }
	}
	
	return 0;
}


static int gpio_dis_event(uint32_t port)
{
    struct epoll_event ep_event;

	if (port >= GPIO_PORT_MAX || gpio_info.init == 0)
		return -1;

	// Remove epol
    epoll_ctl(gpio_info.epoll, EPOLL_CTL_DEL, gpio_setting[port].fd, &ep_event);

	// Remove value of file
	if (gpio_setting[port].fd != -1)
		close(gpio_setting[port].fd);

	// Clear data
	gpio_setting[port].fd       = -1;
	gpio_setting[port].evclback = NULL;
	gpio_setting[port].event    = GPIO_EVENT_NONE;

	// Unexport GPIO
	gpio_file_write_num("/sys/class/gpio/unexport", port);

	return 0;
}

		
static uint8_t gpio_get_port_num(void)
{
	return GPIO_PORT_MAX;
}


int gpio_new(struct gpio_class *gpio_obj)
{
	if (gpio_obj == NULL)
		return -1;

	if (gpio_info.init == 0) {
		
		// Init variables
		memset(&gpio_info, 0, sizeof(gpio_info));
		memset(gpio_setting, 0, sizeof(gpio_setting));
		
		// Open memory file
		gpio_info.fd_map = open("/dev/mem", O_RDWR | O_SYNC);
		if (gpio_info.fd_map < 0) {
			printf("ERROR: couldn't open /dev/mem\n");
			return -1;
		}

		// Mapping
		gpio_info.map_gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpio_info.fd_map, GPIO_BASE_ADDR);
		if (gpio_info.map_gpio == MAP_FAILED) {
			printf("ERROR: call mmap not successfully\n");
			return -1;
		}
		
		gpio_info.init = 1;
	}

	gpio_obj->set_mode     = gpio_set_mode;
	gpio_obj->get_mode     = gpio_get_mode;
	gpio_obj->set_level    = gpio_set_level;
	gpio_obj->get_level    = gpio_get_level;
	gpio_obj->set_pull     = gpio_set_pull;
	gpio_obj->get_pull     = gpio_get_pull;
	gpio_obj->ena_event    = gpio_ena_event;
	gpio_obj->dis_event    = gpio_dis_event;
	gpio_obj->get_port_num = gpio_get_port_num;
	
	return 0;
}


int gpio_free(void)
{
	uint8_t ct;
	
	if (gpio_info.init == 0)
		return -1;

	// Unexport
	for (ct = 0; ct < GPIO_PORT_MAX; ct++)
		gpio_file_write_num("/sys/class/gpio/unexport", ct);
	
	// Unmap
    munmap((void *)gpio_info.map_gpio, BLOCK_SIZE);

	// Close file
    close(gpio_info.fd_map);

	// Init variables
	memset(&gpio_info, 0, sizeof(gpio_info));
	memset(gpio_setting, 0, sizeof(gpio_setting));

	return 0;	
}


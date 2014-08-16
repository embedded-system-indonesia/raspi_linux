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

#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdint.h>							// uint8_t, uint32_t


typedef uint8_t gpio_mode_t;
#define GPIO_MODE_INPUT						(0)
#define GPIO_MODE_OUTPUT					(1)

typedef uint8_t gpio_pull_t;
#define GPIO_PULL_NONE						(0)
#define GPIO_PULL_UP						(1)
#define GPIO_PULL_DOWN						(2)

typedef uint8_t gpio_event_t;
#define GPIO_EVENT_NONE						(0)
#define GPIO_EVENT_RISE_EDGE				(1)
#define GPIO_EVENT_FALL_EDGE				(2)

typedef void (*gpio_ev_callback) (uint32_t port, gpio_event_t event);


struct gpio_class {
	int           (*set_mode)     (uint32_t port, gpio_mode_t mode);
	gpio_mode_t   (*get_mode)     (uint32_t port);
	int           (*set_level)    (uint32_t port, uint8_t level);
	uint8_t       (*get_level)    (uint32_t port);
	int           (*set_pull)     (uint32_t port, gpio_pull_t pull);
	gpio_pull_t   (*get_pull)     (uint32_t port);
	int           (*ena_event)    (uint32_t port, gpio_event_t event, gpio_ev_callback evclback);
	int           (*dis_event)    (uint32_t port);
	uint8_t       (*get_port_num) (void);
};


// Prototype
extern int gpio_new(struct gpio_class *gpio_obj);
extern int gpio_free(void);

#endif


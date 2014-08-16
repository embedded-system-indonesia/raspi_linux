# 
# Copyright (C) 2014, Siryogi Majdi
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details at 
# <http://www.gnu.org/licenses/>.
#

# Define directory variable
SRCDIR = .

# Define project
TARGET = test

# Define prefix
PREFIX = arm-linux-gnueabihf-

#
# Define C sources
# Select file you want to test
#
# CSRC   = 
# CSRC   = $(SRCDIR)/test_gpio.c
# CSRC   = $(SRCDIR)/test_spi.c
# CSRC   = $(SRCDIR)/test_tm1638.c
# CSRC   = $(SRCDIR)/test_remocon.c
CSRC   = $(SRCDIR)/test_i2c.c

# Define Assembly sources
ASRC   = 

# Define library files
LIBS    = 
LIBS   += ../drivers/build/libdriver.a

# Define library folders
INCDIR  =
INCDIR += -I../drivers/include

# Define sub makefile
MAKE_LIB       =
MAKE_SUB       = $(MAKE) -C ../drivers all
MAKE_SUB_CLEAN = $(MAKE) -C ../drivers clean


# Include compilation
include ../rules.mk
	

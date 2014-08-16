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
SRCDIR = src

# Define project
TARGET = libdriver.a

# Define prefix
PREFIX = arm-linux-gnueabihf-

#
# Define C sources
# Comment out if don't want to include in library file !
#
CSRC   = 
CSRC  += $(SRCDIR)/gpio.c
CSRC  += $(SRCDIR)/spi.c
CSRC  += $(SRCDIR)/i2c.c
CSRC  += $(SRCDIR)/tm1638.c
CSRC  += $(SRCDIR)/remocon.c
CSRC  += $(SRCDIR)/ioexp_pcf8574.c

# Define Assembly sources
ASRC   = 

# Define library files
LIBS    =
LIBS   +=

# Define library folders
INCDIR  =
INCDIR += -Iinclude

# Define sub makefile
MAKE_LIB       = Y
MAKE_SUB       = 
MAKE_SUB_CLEAN = 


# Include compilation
include ../rules.mk
	

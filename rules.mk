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

# Various directories
OBJDIR   = $(BUILDDIR)/obj
LSTDIR   = $(BUILDDIR)/lst

# Compiler
CC       = $(PREFIX)gcc
ifeq ($(MAKE_LIB),Y)
LD       = $(PREFIX)ar
LDFLAGS  = 
LIBFLAGS = rcs
OBJFLAG  =
else
LD       = $(PREFIX)gcc
LDFLAGS  = -lpthread 
LIBFLAGS =
OBJFLAG  = -o
endif
AFLAGS   = -Wa,-amhls=$(LSTDIR)/$(notdir $(<:.s=.lst))
CFLAGS   = -Wa,-alms=$(LSTDIR)/$(notdir $(<:.c=.lst))
# CP     = $(PREFIX)objcopy
# OD     = $(PREFIX)objdump
# HEX    = $(CP) -O ihex
# BIN    = $(CP) -O binary


#
# Compilation Area
#

# Define build directory
BUILDDIR = build

# Sources path
SRCPATHS = $(sort $(dir $(ASRC)) $(dir $(CSRC)))
VPATH    = $(SRCPATHS)

# Object files groups
AOBJS    = $(addprefix $(OBJDIR)/, $(notdir $(ASRC:.s=.o)))
COBJS    = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o)))
OBJS     = $(AOBJS) $(COBJS)


#
# Make all
#

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LIBFLAGS) $(OBJFLAG) $(BUILDDIR)/$(TARGET) $^ $(LIBS) $(LDFLAGS)

$(OBJS): | $(BUILDDIR) make_sub

$(BUILDDIR) $(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(LSTDIR)

make_sub:
	$(MAKE_SUB)

# Compile Assembly sources
$(AOBJS) : $(OBJDIR)/%.o : %.s
	@echo
	$(AS) -c $(AFLAGS) $(INCDIR) $< -o $@

# Compile C sources
$(COBJS) : $(OBJDIR)/%.o : %.c
	@echo
	$(CC) -c $(CFLAGS) $(INCDIR) $< -o $@

	
#
# Make clean
#
clean:
	@echo Cleaning
	$(MAKE_SUB_CLEAN)
	find $(CURDIR) -exec touch \{\} \;
	-rm -fR .dep $(BUILDDIR)
	find $(CURDIR) -name *.bak -type f -delete
	@echo Done


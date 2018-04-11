# This file is part of the AVRClock distribution
# (https://github.com/LuTri/AVRClock).
# Copyright (c) 2016 Tristan Lucas.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

# Inherit parameters from a parent's Makefile
ifndef MCU # default target MCU to atmega328p
	MCU = atmega328p
endif

ifndef F_OSC # default quartz speed to 16MHz
	F_OSC = 16000000
endif

ifndef TIMERNR # default timer number to 1
	TIMERNR=1
endif

ifndef PRESCALER # default prescaler to 1024
	PRESCALER=1024
endif

ifndef TIMERBITS # default bitsize of used timer to 16
	TIMERBITS=16
endif

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).
TARGET = customtimer


# List C source files here. (C dependencies are automatically generated.)
SRC = $(TARGET).c

# Optimization level, can be [0, 1, 2, 3, s].
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG = dwarf-2

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99


TIMERFLAGS = -DTIMERNR=$(TIMERNR)
TIMERFLAGS += -DPRESCALER=$(PRESCALER)
TIMERFLAGS += -DTIMERBITS=$(TIMERBITS)

# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += $(CSTANDARD)
CFLAGS += -DF_OSC=$(F_OSC)
CFLAGS += -DF_CPU=$(F_OSC)
CFLAGS += -DMAX_COUNTDOWNS=10
CFLAGS += $(TIMERFLAGS)

TFLAGS = -Wall
TFLAGS += -std=c99
TFLAGS += -DTESTING
TFLAGS += $(TIMERFLAGS)

# Define programs and commands.
CC = avr-gcc
SIZE = avr-size
REMOVE = rm -f

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_COMPILING = Compiling:
MSG_CLEANING = Cleaning project:




# Define all object files.
OBJ = $(SRC:.c=.o)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)

# Default target.
all: begin clean gccversion build size finished end

build: object

size:
	$(SIZE) $(TARGET).o

object: $(TARGET).o

tobject: customtimer.o test.o

begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo

# Display compiler version information.
gccversion :
	@$(CC) --version

# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@

link:
	$(CC) $(ALL_CFLAGS) $(OBJ) -o $(TARGET)

# Target: clean project.
clean: begin clean_list finished end

run_test:
	./test

# Target: run tests
test: TARGET=test
test: SRC=$(TARGET).c customtimer.c
test: CC=gcc
test: ALL_CFLAGS=$(TFLAGS)
test: begin clean gccversion tobject link run_test end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OBJ)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.lst)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) .dep/*



# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all test begin finish end size gccversion build clean clean_list \
   run_test

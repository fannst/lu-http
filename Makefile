# Device Options
MCPU								:= cortex-m7

# Toolchain & Tools
GPP									:= g++
GCC									:= gcc
SIZE								:= size

# General Files
FIRMWARE_ELF					:= firmware.elf

# GCC Arguments
GCC_ARGS						+= -Wall
GCC_ARGS						+= -Werror
GCC_ARGS						+= -pthread
GCC_ARGS						+= -D_GNU_SOURCE
GCC_ARGS						+= -ggdb

GCC_ARGS						+= -I./inc

# GPP Arguments
GPP_ARGS						+= $(GCC_ARGS)

# Size Arguments
SIZE_ARGS						+= --format=gnu
SIZE_ARGS						+= --radix=10
SIZE_ARGS						+= --common
SIZE_ARGS						+= $(FIRMWARE_ELF)

# Files
GPP_SOURCES						+= $(shell find ./src -name *.cc)
C_SOURCES						+= $(shell find ./src -name *.c)
S_SOURCES						+= $(shell find ./src -name *.s)

OBJECTS							+= $(GPP_SOURCES:.cc=.arm.o)
OBJECTS							+= $(C_SOURCES:.c=.arm.o)
OBJECTS							+= $(S_SOURCES:.s=.arm.o)

# Compilation
%.arm.o: %.s
	$(AS) $(AS_ARGS) $< -o $@
%.arm.o: %.cc
	$(GPP) $(GPP_ARGS) -c $< -o $@
%.arm.o: %.c
	$(GCC) $(GCC_ARGS) -c $< -o $@

# General Make Rules
all: $(OBJECTS)
	$(GCC) $(GCC_ARGS) $(OBJECTS) -o $(FIRMWARE_ELF)
size:
	$(SIZE) $(SIZE_ARGS)
clean:
	rm -rf $(OBJECTS) firmware.elf

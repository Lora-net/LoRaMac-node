PROJECT_NAME = $(shell basename "$(realpath ./)")

#List all of the application source files that need to be compiled
#############################################
APPLICATION_SRCS = $(notdir $(wildcard ./*.c))
APPLICATION_SRCS += stm32l1xx_hal_gpio.c
APPLICATION_SRCS += stm32l1xx_hal_rcc_ex.c

LORA_DIR ?= ../../../

#The exact STM part number All CAPS
############################################
DEVICE = STM32L151
DEVICE_FAMILY = STM32L151xB

#The install locations of the STM Standard library
#################################################
#LIBRARY_START_PATH ?= /lib/STM32Cube_FW_L1_V1.6.0/
LIBRARY_START_PATH ?= $(LORA_DIR)src/

#The location of the gcc setup template
################################################
TEMPLATE_PATH ?= $(LORA_DIR)make/example/

#Any places that you put custom header files
###############################################
LIBRARY_PATHS += ../../include

#Any places that you put custom source files
##############################################
SOURCE_PATHS += ../../source

ifndef LIBRARY_START_PATH
$(error An LIBRARY_START_PATH must be defined. Edit Makefile to define a 
	valid LIBRARY_START_PATH)
endif

CFLAGS = -Os
GDB_PORT_NUMBER = 2331

include $(TEMPLATE_PATH)Makefile

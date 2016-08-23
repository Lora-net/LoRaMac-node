PROJECT_NAME = $(shell basename "$(realpath ./)")

#List all of the application source files that need to be compiled
#############################################
CUSTOM_DEFINES = -DUSE_BAND_915
LORA_DIR ?= ../../../../
MAIN_DIR ?= $(LORA_DIR)src/apps/LoRaMac/classA/SK-iM880A/

#do not modify if using SK-iM880A
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/boards/SK-iM880A/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/boards/mcu/stm32/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/mac/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/system/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/system/crypto/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/radio/*.c)
APPLICATION_SRCS += $(wildcard $(LORA_DIR)src/radio/sx1272/*.c)
APPLICATION_SRCS += $(MAIN_DIR)main.c

#The exact STM part number All CAPS
############################################
DEVICE = STM32L151
DEVICE_FAMILY = STM32L151xB

#The install locations of the STM Standard library
#################################################
LIBRARY_START_PATH ?= $(LORA_DIR)src/

#The location of the gcc setup template
################################################
TEMPLATE_PATH ?= $(LORA_DIR)make/example/

#Any places that you put custom header files
###############################################
LIBRARY_PATHS += ../../include
LIBRARY_PATHS += $(LORA_DIR)src/apps/LoRaMac/classA/SK-iM880A/*.h

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

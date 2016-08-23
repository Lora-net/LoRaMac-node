#You shouldn't need to edit this file unless something is wrong
#This file assumes that the standard peripheral library is installed
#currently tested on V1.7 with limited test code

DEVICE_FAMILY_LOWER = $(shell echo $(DEVICE_FAMILY) | tr A-Z a-z)
DEVICE_LOWER = $(shell echo $(DEVICE) | tr A-Z a-z)

START_CODE ?= startup_stm32l1xx_md.s
SYSTEM_FILE ?= system_stm32l1xx.c

LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32/STM32_USB_Device_Library/Core/Inc
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32/STM32_USB_Device_Library/Class/CDC/Inc
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote/cmsis
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote/usb/cdc/inc/
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32/cmsis
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32/STM32L1xx_HAL_Driver/Inc
LIBRARY_PATHS += $(LIBRARY_START_PATH)system
LIBRARY_PATHS += $(LIBRARY_START_PATH)mac
LIBRARY_PATHS += $(LIBRARY_START_PATH)radio 
LIBRARY_PATHS += $(LIBRARY_START_PATH)radio/sx1272 
LIBRARY_PATHS += $(LIBRARY_START_PATH)peripherals 
LIBRARY_PATHS += $(LIBRARY_START_PATH)system/crypto
LIBRARY_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote/

SOURCE_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32/STM32_USB_Device_Library/Core/Src
SOURCE_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote/usb/cdc/src/
SOURCE_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32/STM32L1xx_HAL_Driver/Src
SOURCE_PATHS += $(LIBRARY_START_PATH)boards/mcu/stm32
SOURCE_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote/cmsis
SOURCE_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote
SOURCE_PATHS += $(LIBRARY_START_PATH)boards/LoRaMote/cmsis/arm-gcc
SOURCE_PATHS += $(LIBRARY_START_PATH)mac
SOURCE_PATHS += $(LIBRARY_START_PATH)peripherals
SOURCE_PATHS += $(LIBRARY_START_PATH)radio
SOURCE_PATHS += $(LIBRARY_START_PATH)radio/sx1272
SOURCE_PATHS += $(LIBRARY_START_PATH)system
SOURCE_PATHS += $(LIBRARY_START_PATH)system/crypto

#Setup toolchain
##############################################################################
TOOLCHAIN_PREFIX ?= arm-none-eabi
AS = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-as
CC = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-gcc
LD = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-gcc
OBJCOPY = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-objcopy
OBJDUMP = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-objdump
SIZE = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-size
GDB = $(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)-gdb
###############################################################################

print-% : ; @echo $* = $($*)

LINKER_SCRIPT ?= stm32l1xx_md_flash.ld
LINKER_SCRIPT_INCLUDES = $(LORA_DIR)src/boards/LoRaMote/cmsis/arm-gcc/
OUTPUT_NAME ?= $(PROJECT_NAME)

LIBRARY_INCLUDES = $(addprefix -I,$(LIBRARY_PATHS))
FILE_INCLUDES += ./stm32l1xx_hal_conf.h

VPATH = $(SOURCE_PATHS)

OUTPUT_PATH = _build/

CPUFLAGS = -mthumb -mcpu=cortex-m3 -march=armv7-m
CFLAGS += -std=gnu99 -c $(CPUFLAGS) -nostdlib -Wall -D$(DEVICE_FAMILY) $(LIBRARY_INCLUDES) -s -ffunction-sections -fdata-sections -include $(FILE_INCLUDES) -MD $(CUSTOM_DEFINES)
LDFLAGS += $(CPUFLAGS) -L $(TEMPLATE_PATH) -L$(LINKER_SCRIPT_INCLUDES) -T $(LINKER_SCRIPT) -Wl,--gc-sections -Wl,-Map=$(OUTPUT_PATH)$(OUTPUT_NAME).Map
OBJDUMP_FLAGS += --disassemble-all --source --disassembler-options=force-thumb -C --section-headers


HEX = $(OUTPUT_PATH)$(OUTPUT_NAME).hex
ELF = $(OUTPUT_PATH)$(OUTPUT_NAME).elf
BIN = $(OUTPUT_PATH)$(OUTPUT_NAME).bin
LST = $(OUTPUT_PATH)$(OUTPUT_NAME).lst

SRCS = $(SYSTEM_FILE) $(notdir $(APPLICATION_SRCS))
OBJS = $(addprefix $(OUTPUT_PATH), $(SRCS:.c=.o)) $(addprefix $(OUTPUT_PATH),$(APPLICATION_LIBS))
DEPS = $(addprefix $(OUTPUT_PATH), $(SRCS:.c=.d))
SRCS_AS = $(START_CODE)
OBJS_AS = $(addprefix $(OUTPUT_PATH), $(SRCS_AS:.s=.os))

JLINK_OPTIONS = -if swd -device $(DEVICE) -speed 1000

all: $(OBJS) $(OBJS_AS) $(HEX)

debug: CFLAGS += -g
debug: $(OBJS) $(OBJS_AS) $(HEX)

rebuild: clean all

ifeq ($(OS),Windows_NT)
include $(TEMPLATE_PATH)Makefile.windows
else
include $(TEMPLATE_PATH)Makefile.posix
endif

$(HEX): $(OBJS) $(OBJS_AS)
	$(LD) $(LDFLAGS) $(OBJS_AS) $(OBJS) -o $(ELF) -lm 
	$(OBJCOPY) -Oihex $(ELF) $(HEX)
	$(OBJCOPY) -Obinary $(ELF) $(BIN)
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(ELF) > $(LST)
	$(SIZE) $(ELF)

size: $(ELF)
	$(SIZE) $(ELF)

$(OUTPUT_PATH)%.o: %.c
	$(MAKE_BUILD_FOLDER)
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@

$(OUTPUT_PATH)%.os: %.s
	$(MAKE_BUILD_FOLDER)
	$(AS) $< -o $@ 

-include $(DEPS)

.PHONY: all clean rebuild size

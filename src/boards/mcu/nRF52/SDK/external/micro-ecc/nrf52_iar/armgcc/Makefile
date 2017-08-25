PROJECT_NAME     := ext_micro_ecc_nrf52_library_iar
TARGETS          := micro_ecc_lib
OUTPUT_DIRECTORY := _build

SDK_ROOT := ../../../..
PROJ_DIR := ../..


# Source files common to all targets
SRC_FILES += \
  $(PROJ_DIR)/micro-ecc/uECC.c \

# Include folders common to all targets
INC_FOLDERS += \

# Libraries common to all targets
LIB_FILES += \

# C flags common to all targets
CFLAGS += -DuECC_ENABLE_VLI_API
CFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
CFLAGS += -DuECC_SQUARE_FUNC=1
CFLAGS += -DuECC_SUPPORTS_secp256r1=1
CFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0
CFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS +=  -Wall -Werror -Os -g3
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums -fshort-wchar

# C++ flags common to all targets
CXXFLAGS += \

# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DuECC_ENABLE_VLI_API
ASMFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
ASMFLAGS += -DuECC_SQUARE_FUNC=1
ASMFLAGS += -DuECC_SUPPORTS_secp256r1=1
ASMFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0
ASMFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1



.PHONY: $(TARGETS) default all clean help flash 

# Default target - first one defined
default: micro_ecc_lib

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	micro_ecc_lib

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(call define_library, $(TARGETS), $(PROJ_DIR)/nrf52_iar/armgcc/micro_ecc_lib_nrf52.a)

define create_library
@echo Creating library: $($@)
$(NO_ECHO)$(AR) $($@) $^
@echo Done
endef
micro_ecc_lib:
	$(create_library)

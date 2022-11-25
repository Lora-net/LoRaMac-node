## License:  Revised BSD License, see LICENSE.TXT file included in the project
## Authors:  Alistair Francis <alistair@alistair23.me>
##
## Tock target specific CMake file
##

if(NOT DEFINED LINKER_SCRIPT)
message(FATAL_ERROR "No linker script defined")
endif(NOT DEFINED LINKER_SCRIPT)
message("Linker script: ${LINKER_SCRIPT}")


#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

set(STACK_SIZE 2048)
set(APP_HEAP_SIZE 1024)
set(KERNEL_HEAP_SIZE 1024)

# Object build options
set(OBJECT_GEN_FLAGS "-mthumb -g2 -fno-builtin -mcpu=cortex-m4 -Wall -Wextra -pedantic -Wno-unused-parameter -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize -frecord-gcc-switches -gdwarf-2 -Os -fdata-sections -ffunction-sections -fstack-usage -Wl,--emit-relocs -fPIC -mthumb -mfloat-abi=soft -msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative -D__TOCK__ -DSVCALL_AS_NORMAL_FUNCTION -DSOFTDEVICE_s130")

set(CMAKE_C_FLAGS "${OBJECT_GEN_FLAGS} -std=gnu99 " CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++11 " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -mthumb -g2 -mcpu=cortex-m4 -mabi=aapcs -T${LINKER_SCRIPT} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Xlinker --defsym=STACK_SIZE=${STACK_SIZE} -Xlinker --defsym=APP_HEAP_SIZE=${APP_HEAP_SIZE} -Xlinker --defsym=KERNEL_HEAP_SIZE=${KERNEL_HEAP_SIZE} -nostdlib -Wl,--start-group" CACHE INTERNAL "Linker options")

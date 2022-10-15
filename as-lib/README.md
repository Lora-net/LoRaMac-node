# LoRaMac-node 'as-lib'

**WIP**

## Purpose

This directory is provided to allow LoRaMac-node to be built as a static library for inclusion in other projects.

This directory contains a set of CMakeLists.txt files that shadow the directory structure under `src`. A backwards 
compatible change is made to the project root CMakeLists.txt which, when provided with the option `LORAMAC_AS_LIB`,
will direct CMake to reference the as-lib directory instead of src. The option default is OFF so that the past 
build behaviour is preserved as the default.

CMake configuration options are provided that mirror equivalent options in the `src` configuration tree, but a
standard `LORAMAC_` prefix is applied to all options to aid interoperability with other project sources.

## Supported CMake Configuration Options

- LORAMAC_AS_LIB:BOOL Whether to build the project as a static library (when ON), or to build the example applications
- LORAMAC_SUFFIX:STRING Defaults to empty, but can be set to any string to allow for multiple static libraries to
  be build (for example with different region support)
- LORAMAC_SECURE_ELEMENT:STRING Name of the secure element, defaults to SOFT_SE
- LORAMAC_SECURE_ELEMENT_PRE_PROVISIONED:BOOL Whether the secure element is pre-provisioned (default ON)
- LORAMAC_RADIO:STRING Name of the radio driver, defaults to sx1272
- LORAMAC_USE_RADIO_DEBUG:BOOL Enable Radio Debug GPIO's (default OFF)

## Preparation for loading and building

You must establish your toolchain prior to your first CMake `project()` call (which triggers toolchain detection). It
is beyond the scope of this document to describe how to do that for your platform.

You will need to provide a board implementation that suits your project.  A number of standard boards are provided
in the LoRaMac-node project which can be copied after the first CMake configure pass.  Typically you will copy one of these
(from the `/src/boards` directory) into your own project sources and make any necessary changes.

NB: Pre-built board implementation targets are not provided due to the complexity of providing included or project
specific HAL libraries.  `CMakeLists.txt` files are included in the board implementation directories that can be used
as a starting point for your own project.

Depending on your platform, you may need to obtain Hardware Abstraction Libraries that are used by the board 
implementation that you choose.  LoRaMac-node includes some of these for the provided board implementations (in the
`/src/board/mcu` directory), but you may wish provide your own version (perhaps via FetchContent from the official
sources).

## Configuring via FetchContent to create a single library

```
FetchContent_Declare(
  loramac
  GIT_REPOSITORY https://github.com/Lora-net/LoRaMac-node
  GIT_TAG master  # branch or version tag, such a v4.7.0
)

set(LORAMAC_AS_LIB ON)
set(LORAMAC_RADIO sx1276)
set(REGION_EU868 ON)
FetchContent_MakeAvailable(loramac)

# add your own target

add_executable(MyProject C)
target_sources(MyProject main.c)
target_link_libraries(MyProject loramac)
```

## Configuring via FetchContent to create multiple libraries

FetchContent should be used to load the project at CMake configure time (rather than build time using ExternalProject).

```
FetchContent_Declare(
  loramac
  GIT_REPOSITORY https://github.com/Lora-net/LoRaMac-node
  GIT_TAG master
)

FetchContent_GetProperties(loramac)
if (NOT loramac_POPULATED)
  FetchContent_Populate(loramac)
endif()

set(LORAMAC_AS_LIB ON)
set(LORAMAC_RADIO sx1276)
set(LORAMAC_SUFFIX -Europe)
set(REGION_EU868 ON)
add_subdirectory(loramac_SOURCE_DIR loramac${LORAMAC_SUFFIX})

set(REGION_EU868 OFF)
set(REGION_US915 ON)
set(LORAMAC_SUFFIX -US)
add_subdirectory(loramac_SOURCE_DIR loramac${LORAMAC_SUFFIX})

# You now have targets loramac-Europe and loramac-US to link to your own targets

add_executable(MyProject C)
target_sources(MyProject main.c)
target_link_libraries(MyProject loramac-Europe)
```

## TODO


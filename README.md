# LoRaWAN end-device stack implementation and example projects

      ______                              _
     / _____)             _              | |
    ( (____  _____ ____ _| |_ _____  ____| |__
     \____ \| ___ |    (_   _) ___ |/ ___)  _ \
     _____) ) ____| | | || |_| ____( (___| | | |
    (______/|_____)_|_|_| \__)_____)\____)_| |_|
        (C)2013-2020 Semtech

     ___ _____ _   ___ _  _____ ___  ___  ___ ___
    / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
    \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
    |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
    embedded.connectivity.solutions===============

## Introduction

The aim of this project is to show an example of an end-device LoRaWAN stack implementation.

This project has 2 active branches in place.

| Branch        | L2 spec       | RP spec   | Tag/Milestone       | Class     | Comments      |
| ------------- |:-------------:|:---------:|:---------:|:---------:|:--------------|
| | [1.0.4](https://lora-alliance.org/resource-hub/lorawan-104-specification-package) | [2-1.0.1](https://lora-alliance.org/sites/default/files/2020-02/rp_2-1.0.1.pdf) | [v4.5.1](https://github.com/Lora-net/LoRaMac-node/releases/tag/v4.5.1) | A/B/C | LoRaWAN L2 1.0.4 - **_Released_** |
| | [1.0.3](https://lora-alliance.org/resource-hub/lorawanr-specification-v103) | [v1.0.3revA](https://www.lora-alliance.org/resource-hub/lorawanr-regional-parameters-v103reva) | [v4.4.7](https://github.com/Lora-net/LoRaMac-node/releases/tag/v4.4.7) | A/B/C | LoRaWAN L2 1.0.3 - **_Released_ (last release based on 1.0.3)** |
| [master](https://github.com/Lora-net/LoRaMac-node/tree/master) | [1.0.4](https://lora-alliance.org/resource-hub/lorawan-104-specification-package) | [2-1.0.1](https://lora-alliance.org/sites/default/files/2020-02/rp_2-1.0.1.pdf) | [v4.5.1](https://github.com/Lora-net/LoRaMac-node/releases/tag/v4.5.1) | A/B/C |  LoRaWAN L2 1.0.4 |
| [develop](https://github.com/Lora-net/LoRaMac-node/tree/develop) | [1.0.4](https://lora-alliance.org/resource-hub/lorawan-104-specification-package) / [1.1.1](https://lora-alliance.org/resource-hub/lorawanr-specification-v11) | [2-1.0.1](https://lora-alliance.org/sites/default/files/2020-02/rp_2-1.0.1.pdf) | [M 4.6.0](https://github.com/Lora-net/LoRaMac-node/milestone/3) | A/B/C |  LoRaWAN L2 1.0.4 / 1.1.1 |

This project fully implements ClassA, ClassB and ClassC end-device classes and it also provides SX1272/73, SX1276/77/78/79, SX1261/2 and LR1110 radio drivers.

For each currently supported platform example applications are provided.

* **LoRaMac/fuota-test-01**: FUOTA test scenario 01 end-device example application. (Based on provided application common packages)

* **LoRaMac/periodic-uplink-lpp**: ClassA/B/C end-device example application. Periodically uplinks a frame using the Cayenne LPP protocol. (Based on provided application common packages)

* **ping-pong**: Point to point RF link example application.

* **rx-sensi**: Example application useful to measure the radio sensitivity level using an RF generator.

* **tx-cw**: Example application to show how to generate an RF Continuous Wave transmission.

**Note**: *Each LoRaWAN application example (LoRaMac/\*) includes an implementation of the LoRa-Alliance; LoRaWAN certification protocol.*

**Note**: *The LoRaWAN stack API documentation can be found at: http://stackforce.github.io/LoRaMac-doc/*

## Supported platforms

This project currently provides support for the below platforms.  
This project can be ported to other platforms using different MCU than the ones currently supported.  
The [Porting Guide](http://stackforce.github.io/LoRaMac-doc/_p_o_r_t_i_n_g__g_u_i_d_e.html) document provides guide lines on how to port the project to other platforms.

* NAMote72
  * [NAMote72 platform documentation](doc/NAMote72-platform.md)

* NucleoLxxx - Discovery kit
  * [NucleoLxxx and Discovery kit platforms documentation](doc/NucleoLxxx-platform.md)

* SKiM880B, SKiM980A, SKiM881AXL
  * [SKiM88xx platforms documentation](doc/SKiM88xx-platform.md)

* SAMR34
  * [SAMR34 platform documentation](doc/SAMR34-platform.md)

## Getting Started

### Prerequisites

Please follow instructions provided by [Development environment](doc/development-environment.md) document.

### Cloning the repository

Clone the repository from GitHub

```bash
$ git clone https://github.com/lora-net/loramac-node.git loramac-node
```

LoRaMac-node project contains Git submodules that must be initialized

```bash
$ cd loramac-node
$ git submodule update --init
```

### Secure-element commissioning

This project currently supports 3 different secure-elements `soft-se`, `lr1110-se`
and `atecc608a-tnglora-se` implementations.

In order to personalize the MCU binary file with LoRaWAN EUIs or/and AES128 keys
one must follow the instructions provided by [soft-se](####soft-se),
 [lr1110-se](####lr1110-se) and [atecc608a-tnglora-se](####atecc608a-tnglora-se) chapters

#### soft-se

*soft-se* is a pure software emulation of a secure-element. It means that everything is located on the host MCU memories. The `DevEUI`, `JoinEUI` and `AES128 keys` may be stored on a non-volatile memory through dedicated APIs.

In order to update the end-device identity (`DevEUI`, `JoinEUI` and `AES128 keys`) one must update the `se-identity.h` file located under `./src/peripherals/soft-se/` directory.  

**Note:** In previous versions of this project this was done inside `Commissioning.h` files located under each provided example directory.

#### lr1110-se

*lr1110-se* abstraction implementation handles all the required exchanges with the LR1110 radio crypto-engine.

All LR1110 radio chips are pre-provisioned out of factory in order to be used with [LoRa Cloud Device Join Service](https://www.loracloud.com/documentation/join_service).  

In case other Join Servers are to be used the `DevEUI`, `Pin`, `JoinEUI` and `AES128 keys` can be updated by following the instructions provided on chapter "13. LR1110 Provisioning" of the [LR1110 User Manual](https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R000000Q2PM/KGm1YHDoHhtaicNYHCIAnh0CbG3yodEuWWJ2WrFRafM).

When the compile option `SECURE_ELEMENT_PRE_PROVISIONED` is set to `ON` the *lr1110-se* will use the factory provisioned data (`DevEUI`, `JoinEUI` and `AES128 keys`).  
When the compile option `SECURE_ELEMENT_PRE_PROVISIONED` is set to `OFF` the *lr1110-se* has to be provisioned by following one of the methods described on chapter "13. LR1110 Provisioning" of the [LR1110 User Manual](https://semtech.my.salesforce.com/sfc/p/#E0000000JelG/a/2R000000Q2PM/KGm1YHDoHhtaicNYHCIAnh0CbG3yodEuWWJ2WrFRafM).
The `DevEUI`, `Pin` and `JoinEUI` can be changed by editing the `se-identity.h` file located in `./src/peripherals/lr1110-se/` directory.

#### atecc608a-tnglora-se

The *atecc608a-tnglora-se* abstraction implementation handles all the required exchanges with the ATECC608A-TNGLORA secure-element.

ATECC608A-TNGLORA secure-element is always pre-provisioned and its contents can't be changed.

### Building Process

#### Command line

**periodic-uplink-lpp** example for NucleoL476 platform with LR1110MB1DIS MBED shield and using LR1110 pre-provisioned secure-element

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DTOOLCHAIN_PREFIX="<replace by toolchain path>" \
        -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchain-arm-none-eabi.cmake" \
        -DAPPLICATION="LoRaMac" \
        -DSUB_PROJECT="periodic-uplink-lpp" \
        -DCLASSB_ENABLED="ON" \
        -DACTIVE_REGION="LORAMAC_REGION_EU868" \
        -DREGION_EU868="ON" \
        -DREGION_US915="OFF" \
        -DREGION_CN779="OFF" \
        -DREGION_EU433="OFF" \
        -DREGION_AU915="OFF" \
        -DREGION_AS923="OFF" \
        -DREGION_CN470="OFF" \
        -DREGION_KR920="OFF" \
        -DREGION_IN865="OFF" \
        -DREGION_RU864="OFF" \
        -DBOARD="NucleoL476" \
        -DMBED_RADIO_SHIELD="LR1110MB1XXS" \
        -DSECURE_ELEMENT="LR1110_SE" \
        -DSECURE_ELEMENT_PRE_PROVISIONED="ON" \
        -DUSE_RADIO_DEBUG="ON" ..
$ make
```

**ping-pong** example using LoRa modulation for NucleoL476 platform with LR1110MB1DIS MBED shield

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DTOOLCHAIN_PREFIX="<replace by toolchain path>" \
        -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchain-arm-none-eabi.cmake" \
        -DAPPLICATION="ping-pong" \
        -DMODULATION:"LORA" \
        -DREGION_EU868="ON" \
        -DREGION_US915="OFF" \
        -DREGION_CN779="OFF" \
        -DREGION_EU433="OFF" \
        -DREGION_AU915="OFF" \
        -DREGION_AS923="OFF" \
        -DREGION_CN470="OFF" \
        -DREGION_KR920="OFF" \
        -DREGION_IN865="OFF" \
        -DREGION_RU864="OFF" \
        -DBOARD="NucleoL476" \
        -DMBED_RADIO_SHIELD="LR1110MB1XXS" \
        -DUSE_RADIO_DEBUG="ON" ..
$ make
```

#### VSCode

**periodic-uplink-lpp** example for NucleoL476 platform with LR1110MB1DIS MBED shield and using LR1110 pre-provisioned secure-element

* Please edit .vscode/settings.json file

<details>
  <summary>Click to expand!</summary>
<p>

```json
// Place your settings in this file to overwrite default and user settings.
{
    "cmake.configureSettings": {

        // In case your GNU ARM-Toolchain is not installed under the default
        // path:
        //     Windows : No default path. Specify the path where the
        //               toolchain is installed. i.e:
        //               "C:/PROGRA~2/GNUTOO~1/92019-~1".
        //     Linux   : /usr
        //     OSX     : /usr/local
        // It is required to uncomment and to fill the following line.
        "TOOLCHAIN_PREFIX":"/path/to/toolchain",

        // In case your OpenOCD is not installed under the default path:
        //     Windows : C:/openocd/bin/openocd.exe
        //     Linux   : /usr/bin/openocd
        //     OSX     : /usr/local/bin/openocd
        // Please uncomment the following line and fill it accordingly.
        //"OPENOCD_BIN":"C:/openocd/bin/openocd.exe",

        // Specifies the path to the CMAKE toolchain file.
        "CMAKE_TOOLCHAIN_FILE":"cmake/toolchain-arm-none-eabi.cmake",

        // Determines the application. You can choose between:
        // LoRaMac (Default), ping-pong, rx-sensi, tx-cw.
        "APPLICATION":"LoRaMac",

        // Select LoRaMac sub project. You can choose between:
        // periodic-uplink-lpp, fuota-test-01.
        "SUB_PROJECT":"periodic-uplink-lpp",

        // Switch for Class B support of LoRaMac:
        "CLASSB_ENABLED":"ON",

        // Select the active region for which the stack will be initialized.
        // You can choose between:
        // LORAMAC_REGION_EU868, LORAMAC_REGION_US915, ..
        "ACTIVE_REGION":"LORAMAC_REGION_EU868",

        // Select the type of modulation, applicable to the ping-pong or
        // rx-sensi applications. You can choose between:
        // LORA or FSK
        "MODULATION":"LORA",

        // Target board, the following boards are supported:
        // NAMote72, NucleoL073 (Default), NucleoL152, NucleoL476, SAMR34, SKiM880B, SKiM980A, SKiM881AXL, B-L072Z-LRWAN1.
        "BOARD":"NucleoL476",

        // MBED Radio shield selection. (Applies only to Nucleo platforms)
        // The following shields are supported:
        // SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS(Default), SX1262MBXCAS, SX1262MBXDAS, LR1110MB1XXS.
        "MBED_RADIO_SHIELD":"LR1110MB1XXS",

        // Secure element type selection the following are supported
        // SOFT_SE(Default), LR1110_SE, ATECC608A_TNGLORA_SE
        "SECURE_ELEMENT":"LR1110_SE",

        // Secure element is pre-provisioned
        "SECURE_ELEMENT_PRE_PROVISIONED":"ON",

        // Region support activation, Select the ones you want to support.
        // By default only REGION_EU868 support is enabled.
        "REGION_EU868":"ON",
        "REGION_US915":"OFF",
        "REGION_CN779":"OFF",
        "REGION_EU433":"OFF",
        "REGION_AU915":"OFF",
        "REGION_AS923":"OFF",
        "REGION_CN470":"OFF",
        "REGION_KR920":"OFF",
        "REGION_IN865":"OFF",
        "REGION_RU864":"OFF",
        "USE_RADIO_DEBUG":"ON"
    }
}
```

</p>
</details>

* Click on "CMake: Debug: Ready" and select build type Debug or Release.  
![cmake configure](doc/images/vscode-cmake-configure.png)
* Wait for configuration process to finish
* Click on "Build" to build the project.  
![cmake build](doc/images/vscode-cmake-build.png)
* Wait for build process to finish
* Binary files will be available under `./build/src/apps/LoRaMac/`
  * LoRaMac-periodic-uplink-lpp     - elf format
  * LoRaMac-periodic-uplink-lpp.bin - binary format
  * LoRaMac-periodic-uplink-lpp.hex - hex format

**ping-pong** example using LoRa modulation for NucleoL476 platform with LR1110MB1DIS MBED shield

* Please edit .vscode/settings.json file

<details>
  <summary>Click to expand!</summary>
<p>

```json
// Place your settings in this file to overwrite default and user settings.
{
    "cmake.configureSettings": {

        // In case your GNU ARM-Toolchain is not installed under the default
        // path:
        //     Windows : No default path. Specify the path where the
        //               toolchain is installed. i.e:
        //               "C:/PROGRA~2/GNUTOO~1/92019-~1".
        //     Linux   : /usr
        //     OSX     : /usr/local
        // It is required to uncomment and to fill the following line.
        "TOOLCHAIN_PREFIX":"/path/to/toolchain",

        // In case your OpenOCD is not installed under the default path:
        //     Windows : C:/openocd/bin/openocd.exe
        //     Linux   : /usr/bin/openocd
        //     OSX     : /usr/local/bin/openocd
        // Please uncomment the following line and fill it accordingly.
        //"OPENOCD_BIN":"C:/openocd/bin/openocd.exe",

        // Specifies the path to the CMAKE toolchain file.
        "CMAKE_TOOLCHAIN_FILE":"cmake/toolchain-arm-none-eabi.cmake",

        // Determines the application. You can choose between:
        // LoRaMac (Default), ping-pong, rx-sensi, tx-cw.
        "APPLICATION":"ping-pong",

        // Select LoRaMac sub project. You can choose between:
        // periodic-uplink-lpp, fuota-test-01.
        "SUB_PROJECT":"periodic-uplink-lpp",

        // Switch for Class B support of LoRaMac:
        "CLASSB_ENABLED":"ON",

        // Select the active region for which the stack will be initialized.
        // You can choose between:
        // LORAMAC_REGION_EU868, LORAMAC_REGION_US915, ..
        "ACTIVE_REGION":"LORAMAC_REGION_EU868",

        // Select the type of modulation, applicable to the ping-pong or
        // rx-sensi applications. You can choose between:
        // LORA or FSK
        "MODULATION":"LORA",

        // Target board, the following boards are supported:
        // NAMote72, NucleoL073 (Default), NucleoL152, NucleoL476, SAMR34, SKiM880B, SKiM980A, SKiM881AXL, B-L072Z-LRWAN1.
        "BOARD":"NucleoL476",

        // MBED Radio shield selection. (Applies only to Nucleo platforms)
        // The following shields are supported:
        // SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS(Default), SX1262MBXCAS, SX1262MBXDAS, LR1110MB1XXS.
        "MBED_RADIO_SHIELD":"SX1261MBXBAS",

        // Secure element type selection the following are supported
        // SOFT_SE(Default), LR1110_SE, ATECC608A_TNGLORA_SE
        "SECURE_ELEMENT":"SOFT_SE",

        // Secure element is pre-provisioned
        "SECURE_ELEMENT_PRE_PROVISIONED":"ON",

        // Region support activation, Select the ones you want to support.
        // By default only REGION_EU868 support is enabled.
        "REGION_EU868":"ON",
        "REGION_US915":"OFF",
        "REGION_CN779":"OFF",
        "REGION_EU433":"OFF",
        "REGION_AU915":"OFF",
        "REGION_AS923":"OFF",
        "REGION_CN470":"OFF",
        "REGION_KR920":"OFF",
        "REGION_IN865":"OFF",
        "REGION_RU864":"OFF",
        "USE_RADIO_DEBUG":"ON"
    }
}
```

</p>
</details>

* Click on "CMake: Debug: Ready" and select build type Debug or Release.  
![cmake configure](doc/images/vscode-cmake-configure.png)
* Wait for configuration process to finish
* Click on "Build" to build the project.  
![cmake build](doc/images/vscode-cmake-build.png)
* Wait for build process to finish
* Binary files will be available under `./build/src/apps/ping-pong/`
  * ping-pong     - elf format
  * ping-pong.bin - binary format
  * ping-pong.hex - hex format

## Acknowledgments

* The mbed (https://mbed.org/) project was used at the beginning as source of
inspiration.
* This program uses the AES algorithm implementation (http://www.gladman.me.uk/) by Brian Gladman.
* This program uses the CMAC algorithm implementation
(http://www.cse.chalmers.se/research/group/dcs/masters/contikisec/) by Lander Casado, Philippas Tsigas.
* [The Things Industries](https://www.thethingsindustries.com/) for providing
 Microchip/Atmel SAMR34 platform and ATECC608A-TNGLORA secure-element support.
* Tencent Blade Team for security breach findings and solving propositions.

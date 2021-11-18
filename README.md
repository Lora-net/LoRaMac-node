# Picotracker Lora
This is the code repo for the picotracker LoRa. It is based off the Loramac-node v4.4.7 release. It uses Lorawan version 1.0.3 and Regional Parameters v1.0.3revA. The code has the following new features:
 * Ability to switch LoRaWAN regional parameters depending on loramac region(determined by GPS position)
 * Alternates between two networks, The Things Network and the Helium Network.
 * Saves 520 past positions in EEPROM and sends 13 of them down in each transmission.
 * Adds Unit tests with CppUTest.
 * Adds driver for a U-blox MAX-M8C/Q GPS module. Written to ensure GPS and MCU don't run at same time, to minimise peak current consumption.
 * Adds Watchdog

The hardware designs remain the same as the ones in the old respository: https://github.com/ImperialSpaceSociety/picotracker-Lora

## Uploading the code to the picotracker
Install STM32CubeProg which you can download from here: https://www.st.com/en/development-tools/stm32cubeprog.html. There are versions for Windows and Linux.

Then connect the programmer to the tracker. Then hit the connect button as shown below:
![image](https://user-images.githubusercontent.com/26815217/142339814-2babee0e-61d1-4fcb-9b9b-6bab52a627bb.png)


Hit the read button(see below) to verify that the connection is working. This should read out the memory of the tracker.![image](https://user-images.githubusercontent.com/26815217/134256899-17072a9c-32f3-4b58-9ee3-595b4cd913c1.png)

Now switch to the `Erasing and Programming` tab as shown in the screen shot below. Then hit the browse button to open the hex file from your storage drive that you want flash to the tracker.
![image](https://user-images.githubusercontent.com/26815217/134257391-f6bd190f-be0c-4ce4-8083-5bddb161cea9.png)

Ensure the checkbox to `Verify Programming` checkbox is selected and then hit the `Start programming` button. It should flash the code into the tracker MCU. See image below for reference.
![image](https://user-images.githubusercontent.com/26815217/134257621-5bfe6cdb-1644-4bbf-bf76-9e4966002559.png)

Now the code should have been flashed!

## Notes on updating the flash firmware on the Ublox module.
If you have a module with FW version older than FW3.01, you may be able to update it. The instructions to do so, and the firmware binary, can be found in this link: https://junipersys.com/data/support/mesa-2-ublox-gnss-firmware-update.pdf


## Modifying the mac settings on Things Network

Install ttn-cli with the following commands(documentation: https://www.thethingsindustries.com/docs/getting-started/cli/installing-cli/)
```bash
sudo snap install ttn-lw-stack
sudo snap alias ttn-lw-stack.ttn-lw-cli ttn-lw-cli
```
Now proceed to the root of this directory. Here you will have a file called `ttn-lw-cli.yml` that will point to the right TTN server. The contents
look like this:
```bash
oauth-server-address: 'https://eu1.cloud.thethings.network/oauth'
identity-server-grpc-address: 'eu1.cloud.thethings.network:8884'

gateway-server-grpc-address: 'eu1.cloud.thethings.network:8884'
network-server-grpc-address: 'eu1.cloud.thethings.network:8884'
application-server-grpc-address: 'eu1.cloud.thethings.network:8884'
join-server-grpc-address: 'eu1.cloud.thethings.network:8884'
device-claiming-server-grpc-address: 'eu1.cloud.thethings.network:8884'
device-template-converter-grpc-address: 'eu1.cloud.thethings.network:8884'
qr-code-generator-grpc-address: 'eu1.cloud.thethings.network:8884'
```

Note that it will be a different address if you use the US/AU servers. This is the address of the EU servers.

Then do an authentication. It will open up the browser to key in the things network credentials. 

```bash
ttn-lw-cli login -c ttn-lw-cli.yml
```

You will see an output like this.

```bash
medad@medad-ThinkPad-P51:~/Documents/GitHub/LoRaMac-node$ ttn-lw-cli login -c ttn-lw-cli.yml
INFO	Opening your browser on https://eu1.cloud.thethings.network/oauth/authorize?client_id=cli&redirect_uri=local-callback&response_type=code
WARN	Could not open your browser, you'll have to go there yourself	{"error": "fork/exec /usr/bin/xdg-open: permission denied"}
INFO	After logging in and authorizing the CLI, we'll get an access token for future commands.
INFO	Waiting for your authorization...
INFO	Successfully got an access token.
```
Now you can update the settings of your devices. An example below sets the period between device status requests to 20000 hours(over 2 years)
```bash
ttn-lw-cli end-devices set --application-id "icss-lora-tracker" --device-id "icspace26-hab-eu-863-870" --mac-settings.status-time-periodicity 20000h2m3s -c ttn-lw-cli.yml
```

An example below sets the period between device status requests to 20000 hours(over 2 years)
```bash
ttn-lw-cli end-devices set --application-id "icss-lora-tracker" --device-id "icspace26-hab-eu-863-870" --mac-settings.status-time-periodicity 20000h2m3s -c ttn-lw-cli.yml
```

Set the fcount size option with:
```bash
ttn-lw-cli end-devices set --application-id "icss-lora-tracker" --device-id "icspace26-hab-eu-863-870" --mac-settings.supports-32-bit-f-cnt true -c ttn-lw-cli.yml

```

Set Rx1 delay to 1 second
```bash
ttn-lw-cli end-devices set --application-id "icss-lora-tracker" --device-id "icspace26-eu1-eu-863-870-device-5"  --mac-settings.desired-rx1-delay RX_DELAY_1 -c ttn-lw-cli.yml
```


## Development Environment and Unittesting

Here are the instructions for compiling the unittests in this project. Some of the unittests include:
1. Testing geofencing
2. testing compression of NVM data with LZ4

So far, all development has been tested only on Ubuntu 20.01 desktop. First, install [Visual Studio code](https://code.visualstudio.com/). Then clone this project:
```bash
git clone https://github.com/ImperialSpaceSociety/LoRaMac-node.git
```

Then open the project folder in VS code. Install all the recommended extensions specified in `.vscode/extensions.json`. IMPORTANT: downgrade the CppuTest Adaptor extension(`bneumann.cpputest-test-adapter`) to version `v0.1.6`. Later versions have a bug where it cannot parse test output that has printouts in them.

Shown below is the interface you will use to run the tests.
1. Set the compiler to your machine's GCC compiler.
2. Set the target to Unittest_build to generate the make files.
3. Build the project.
4. Hit the play button on the test adaptor to run all tests.
![image](https://user-images.githubusercontent.com/26815217/135118102-049eb3b5-659b-4323-9385-58118ac69132.png)

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
| | [1.0.3](https://lora-alliance.org/resource-hub/lorawanr-specification-v103) | [v1.0.3revA](https://www.lora-alliance.org/resource-hub/lorawanr-regional-parameters-v103reva) | [v4.4.7](https://github.com/Lora-net/LoRaMac-node/releases/tag/v4.4.7) | A/B/C | LoRaWAN L2 1.0.3 - **_Released_ (last release based on 1.0.3)** |
| [master](https://github.com/Lora-net/LoRaMac-node/tree/master) | [1.0.4](https://lora-alliance.org/resource-hub/lorawan-104-specification-package) | [2-1.0.1](https://lora-alliance.org/sites/default/files/2020-02/rp_2-1.0.1.pdf) | [v4.5.0](https://github.com/Lora-net/LoRaMac-node/releases/tag/v4.5.0) | A/B/C |  LoRaWAN L2 1.0.4 |
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

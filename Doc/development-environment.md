# Introduction

This project uses CMake and the GNU ARM-Toolchain as build system and GDB/OpenOCD are used for debugging purposes.

By using these tools the development environment is platform agnostic and independent of chip manufacturer specific Integrated Development Environments.  
It allows to build the project either by using a command line terminal or by using IDE's like [VSCode](#vscode) or [KDevelop](#kdevelop).

# Prerequisites

* CMake >= 3.6
  * GNU/Linux:
    * Ubuntu 16.04/ Linux Mint 18: Since the official repository version is too old, one can use e.g. [PPA](https://launchpad.net/~adrozdoff/+archive/ubuntu/cmake)
    * Linux Arch: `pacman -S cmake`
  * Windows:
    * [CMake Download](https://cmake.org/download/)  
     **Note**: Please use the latest full release and ensure that CMake path is added to the system path variable. (On Windows 10 search for _Environment Variables_ at start menu and add your CMake installation path e.g. `C:\Program Files\CMake\bin` to `Path` variable )
  * OSX:
    * Homebrew: `brew install cmake`
* GNU ARM-Toolchain
  * GNU/Linux:
    * Ubuntu 16.04/ Linux Mint 18: Since the official repository version is too old, one can use e.g. [PPA](https://launchpad.net/~team-gcc-arm-embedded/+archive/ubuntu/ppa)
    * Ubuntu 18.04: the toolchain has been updated but there is a bug with [`libnewlib`](https://github.com/bbcmicrobit/micropython/issues/514#issuecomment-404759614) causing the linker to fail. `sudo apt install gcc-arm-none-eabi`
    * Linux Arch: `pacman -S arm-none-eabi-gcc arm-none-eabi-newlib`
  * Windows:
    * [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
    * The Make utility is also required, one can use e.g. [MSYS2](http://www.msys2.org) or [MinGW](http://mingw.org/)
        * With MSYS2:
            * Follow the installation instructions provided by them.
            * Install MinGW Make with `pacman -S mingw-w64-x86_64-make`.

     **Note**: Please ensure that both paths are added to the system PATH variable. Add for example: `C:\msys64\mingw64\bin` and `C:\Program Files (x86)\GNU Tools ARM Embedded\6 2017-q2-update\bin` to `Path` variable.
  * OSX:
    * Homebrew: `brew tap ARMmbed/homebrew-formulae && brew install arm-none-eabi-gcc`
* OpenOCD
  * GNU/Linux:
    * Ubuntu 16.04/ Linux Mint 18: `apt-get install openocd`
    * Linux Arch: `pacman -S openocd`
  * Windows:
    * Unofficial binary packages are available [here](http://openocd.org/getting-openocd/) for download  
    **Note**: The debug configuration for [VSCode](#vscode) will assume that OpenOCD is installed under `C:/openocd`. If it is not the case one must change the `OPENOCD_BIN` variable according to right location.
  * OSX:
    * Homebrew: `brew install openocd`

# Commandline build instructions

1. Go to root directory of the project

    `cd path/to/project/directory`

2. Create a directory named 'build'

    `mkdir build`

3. Go to the created `build` directory

    `cd build`

4. run

    `cmake -DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-arm-none-eabi.cmake" ..`

**Note**: If the GNU ARM-Toolchain is not installed under the default path (GNU Linux:`/usr`, Mac OS `/usr/local`) a prefix has to be provided:  
    `cmake -DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-arm-none-eabi.cmake" -DTOOLCHAIN_PREFIX="/path/to/the/toolchain" ..`  
For Windows platforms the prefix has to be provided anyway and additionally the CMake Generator for MinGW Makefiles has to be chosen:  
    `cmake -DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-arm-none-eabi.cmake" -DTOOLCHAIN_PREFIX="/path/to/the/toolchain" -G "MinGW Makefiles" ..`  

## Available configuration options for CMake

The possibility to choose the application, target board and more options can be done using the provided configuration options.

These configuration options can be set through additional commandline parameters, for example:  
    `cmake -DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-arm-none-eabi.cmake" -DAPPLICATION="LoRaMac" -DSUB_PROJECT="classC" ..`

Alternatively one can use a graphical interface to configure CMake, drop down menus and check boxes will provide to the user the possible options.

* CMake QT GUI with `cmake-gui ..`
* CMake curses interface with `ccmake ..`

### Options that can be choose by the user

* `APPLICATION` - Application example choice.  
   The possible choices are:  
     * LoRaMac (Default)
     * ping-pong
     * rx-sensi
     * tx-cw
* `SUB_PROJECT` - LoRaMac sub project example choice.  
   **Note**: Only applicable to LoRaMac `APPLICATION` choice.  
   The possible choices are:  
     * classA
     * classB
     * classC
     * periodic-uplink-lpp
     * fuota-test-01
* `ACTIVE_REGION` - Active region for which the stack will be initialized.  
   **Note**: Only applicable to LoRaMac `APPLICATION` choice.  
   The possible choices are:
     * LORAMAC_REGION_EU868
     * LORAMAC_REGION_US915
     * LORAMAC_REGION_CN779
     * LORAMAC_REGION_EU433
     * LORAMAC_REGION_AU915
     * LORAMAC_REGION_AS923
     * LORAMAC_REGION_CN470
     * LORAMAC_REGION_KR920
     * LORAMAC_REGION_IN865
     * LORAMAC_REGION_RU864
* `MODULATION` - Type of modulation choice.  
   **Note**: Only applicable to ping-pong or rx-sensi `APPLICATION` choice.  
   The possible choices are:
     * LORA
     * FSK
* `USE_DEBUGGER`- Enables debugger support. (Default ON)
* `BOARD` - Target board choice.  
   The possible choices are:  
     * NAMote72
     * NucleoL073 (default)
     * NucleoL152
     * NucleoL476
     * SAMR34
     * SKiM880B
     * SKiM980A
     * SKiM881AXL
* `REGION_EU868` - Enables support for the Region EU868 (Default ON)
* `REGION_US915` - Enables support for the Region US915 (Default OFF)
* `REGION_CN779` - Enables support for the Region CN779 (Default OFF)
* `REGION_EU433` - Enables support for the Region EU433 (Default OFF)
* `REGION_AU915` - Enables support for the Region AU915 (Default OFF)
* `REGION_AS923` - Enables support for the Region AS923 (Default OFF)
* `REGION_CN470` - Enables support for the Region CN470 (Default OFF)
* `REGION_KR920` - Enables support for the Region IN865 (Default OFF)
* `REGION_IN865` - Enables support for the Region AS923 (Default OFF)
* `REGION_RU864` - Enables support for the Region RU864 (Default OFF)

### Options that are automatically set

* `RADIO` - Defines the radio to be used.  
   The possible choices are:  
     * sx1272
     * sx1276
* `LINKER_SCRIPT` - Defines the target specific linker script path.
* `OPENOCD_BIN` - Defines the OpenOCD path.
* `OPENOCD_INTERFACE` - Defines the interface configuration file to be used by OpenOCD.
* `OPENOCD_TARGET` - Defines the target configuration file to be used by OpenOCD.

# Debugging

1. OpenOCD  
    OpenOCD has to be started with parameters that depend on the used debugger device and target board.  
    Some examples are shown below:
    * NucleoL073 + STLinkV2-1 (On board debugger):  
    `openocd -f interface/stlink-v2-1.cfg  -f target/stm32l0.cfg`

    * SAMR34 Xplained Pro (On board debugger, tested with openocd 0.10, did not work with 0.9):
    `openocd -f interface/cmsis-dap.cfg -f target/at91samdXX.cfg`

2. GDB  
    The below GDB usage example shows how to start a debug session, writing the program to the flash and run.
   * Run ARM-GNU GDB with:  
   `arm-none-eabi-gdb`
   * Choose the program you want to debug:  
   `file src/apps/LoRaMac/LoRaMac-classA`
   * Connect GDB to OpenOCD:  
   `target extended-remote localhost:3333`
   * Execute a reset and halt of the target:  
   `monitor reset halt`
   * Flash the program to the target Flash memory:  
   `load`
   * Add a one-time break point at main:  
   `thbreak main`
   * Run the program until the break point:  
   `continue`
   * Finally run the program:  
   `continue`


# IDE Support

## VSCode

### Additional Prerequisites

* Visual Studio Code:
  * GNU/Linux, Windows and OSX:
    * [Download](https://code.visualstudio.com/Download)
* Extensions: Open `VSCode ->EXTENSION (Crtl+Shift+x)` and search for:
  * C/C++
  * CMake
  * CMake Tools
  * Native Debug

### Configuration

For Windows platforms it is necessary to make some additional configurations. Open your settings under *File->Preferences->Settings* and add the following lines:

Add MinGW Makefiles as preferred Generator:

```json
"cmake.preferredGenerators": [
        "MinGW Makefiles",
        "Ninja",
        "Unix Makefiles"
    ]
```

Set the CMake path:
```json
    "cmake.cmakePath": "path/to/cmake.exe"
```

### Usage

1. Open the directory of the cloned repository.  
   The *CMake Tools* extension will automatically generate a *`.cmaketools.json`* file based on the CMakeLists.
2. The *`settings.json`* file under `.vscode` directory is the place where one can change the build options.  
   These are the build options that will be provided to CMake.  
   Please see chapter [Commandline Build Instructions](#commandline-build-instructions) for information about build options.
3. Click on the blue status bar of *CMake Tools* to choose a build type (`Debug` or `Release`).  
   A CMake configuration process will be performed.
4. A `Build` button will now be available.  
   Click this button to build the target.
5. The CMake build system will automatically generate a *`launch.json`* file which setups the debugging process for the given board.
6. Press the `F5` key to start a debug session.  
   This will automatically start the GDB and OpenOCD processes.

### Useful Hints

* Change CMake options:  Open the Command palette (Crtl+Shift+P) and type `CMake: Edit the CMake Cache`
* Execute a clean rebuild: Open the Command palette (Crtl+Shift+P) and type `CMake: Clean rebuild`

For detailed information about CMake Tools extension please see their [github repository](https://github.com/vector-of-bool/vscode-cmake-tools).

## KDevelop

### Additional Prerequisites

* KDevelop:
  * GNU/Linux:
    * Ubuntu 16.04/ Linux Mint 18: `apt-get install kdevelop`
    * Linux Arch: `pacman -S kdevelop`
  * Windows:
    * [KDevelop Download](https://www.kdevelop.org/download)  
    **Note**: Currently there is no GDB support but it is planned for future releases.
  * OSX:
    * [KDevelop Download](https://www.kdevelop.org/download).  
    No official binaries are available. Must be built from source code.

### Usage

1. To open the project click on *`Project->Open /Import Project...`* and choose the top level `CMakeLists.txt` directory of the cloned repository.  
   Follow the indications to setup the project and add `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake` to the *`Extra Arguments`*.
2. The CMake options and variables can be changed by right-clicking on project and selecting *`Open configuration...`*.  
   A graphical interface will pop-up.  
   Please see chapter [Commandline Build Instructions](#commandline-build-instructions) for information about build options.
3. Click on `Build` to build the project.
4. Create a launch configuration for debugging:
   * Click on *`Run->Configure Launches...`* and add a new *`Compiled Binary Launcher`*.
   * Set the field *`Debugger executable`* according to your system. For example `/usr/bin/arm-none-eabi-gdb` .
   * Choose the `Run gdb script` according to the application you want to debug.  
     For example: `loramac-node/build/src/apps/LoRaMac/openocd-run.gdb`.  
     **Note**: The CMake build system will automatically generate the GDB run script.
5. Start OpenOCD in a command line terminal as described on chapter [Debugging](#debugging).
6. Click on "Debug" to launch a debug session.

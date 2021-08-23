run the following command to install dependencies


```bash

sudo apt-get install cmake
sudo apt install gcc-arm-none-eabi
sudo apt install openocd
sudo apt-get install -y gdb-multiarch # for gdb
sudo apt install lcov # for coverage reports of cpputest
sudo apt install gcov # for coverage reports of cpputest
sudo apt install gcc-multilib g++-multilib # To compile for 32 bit

```

Then Install vs code.
Install the following extensions:
```
C/C++
CMake
CMake Tools
Cortex-Debug
...


Follow this tutorial to allow stlink to connect
https://electromake.pl/how-to-configure-ubuntu-to-program-stm32/



Check if the st link is discoverable
```bash
sudo apt install stlink-tools
st-info --probe

```
output should look like this
```bash
medad@medad-ThinkPad-P51:~$ st-info --probe
Found 1 stlink programmers
 serial: 323705012612344d314b4e00
openocd: "\x32\x37\x05\x01\x26\x12\x34\x4d\x31\x4b\x4e\x00"
  flash: 196608 (pagesize: 128)
   sram: 20480
 chipid: 0x0447
  descr: L0x Category 5 device
```


Now then you should ping the chip:

```bash
medad@medad-ThinkPad-P51:~$ st-flash read dummy.bin 0 0xFFFF
st-flash 1.6.0
2021-07-15T23:19:26 INFO usb.c: -- exit_dfu_mode
2021-07-15T23:19:26 INFO common.c: Loading device parameters....
2021-07-15T23:19:26 INFO common.c: Device connected is: L0x Category 5 device, id 0x20086447
2021-07-15T23:19:26 INFO common.c: SRAM size: 0x5000 bytes (20 KiB), Flash: 0x30000 bytes (192 KiB) in pages of 128 bytes
```

in launch.json, had to set "debugServerArgs": "-f interface/stlink-v2.cfg -f target/stm32l0.cfg"

not stlink-v2-1.cfg

had to set "miDebuggerPath": "gdb-multiarch",

Run Openocd to get debug info on chip:
```bash
medad@medad-ThinkPad-P51:~$ "/usr/bin/openocd" -f interface/stlink-v2.cfg -f target/stm32l0.cfg
Open On-Chip Debugger 0.10.0
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
Info : auto-selecting first available session transport "hla_swd". To override use 'transport select <transport>'.
adapter speed: 300 kHz
adapter_nsrst_delay: 100
Info : The selected transport took over low-level target control. The results might differ compared to plain JTAG/SWD
none separate
Info : Unable to match requested speed 300 kHz, using 240 kHz
Info : Unable to match requested speed 300 kHz, using 240 kHz
Info : clock speed 240 kHz
Info : STLINK v2 JTAG v37 API v2 SWIM v7 VID 0x0483 PID 0x3748
Info : using stlink api v2
Info : Target voltage: 3.275067
Info : stm32l0.cpu: hardware has 4 breakpoints, 2 watchpoints
```


Now for serial output, install arduino with ubuntu Software.

Then run to use usb
```bash
sudo usermod -a -G dialout $USER
```

Then restart the computer. You should be able to see data come through on the serial terminal.
It could look like this:
```

###### ===================================== ######

Application name   : periodic-uplink-lpp
Application version: 1.0.0
GitHub base version: 4.4.7

###### ===================================== ######

DevEui      : 60-81-F9-99-C0-06-CC-BC
JoinEui     : 60-81-F9-C2-62-7E-35-8C
Pin         : 00-00-00-00


###### =========== MLME-Request ============ ######
######               MLME_JOIN               ######
###### ===================================== ######
STATUS      : OK

###### =========== MLME-Confirm ============ ######
STATUS      : Rx 2 timeout

###### =========== MLME-Request ============ ######
######               MLME_JOIN               ######
###### ===================================== ######
STATUS      : OK

```


## Erase chip + eeprom

Run the following command: 
```bash
python3 -m pyocd pack install stm32l073 # installs device pack. Run only once.
python3 -m pyocd erase -t STM32L073CZYx --chip
```

Dump eeprom with:
```bash
st-flash read eeprom.hex 0x08080000 0x17FF # size of eeprom
```

location of target files:
```bash
/usr/share/openocd/scripts/target
```

### Running unit tests
To run unittests, on vs code, change current build variant from debug to Unittest_build. Then change active kit to GCC x.x.x x86_64-linux-gnu (your computer's gcc compiler). Then hit build. To go back to building for the MCU, switch back current build variant to debug and change active kit to arm-none-eabi

### Running coverage reports
Run the following command in terminal:
```bash
medad@medad-ThinkPad-P51:~/Documents/GitHub/LoRaMac-node/build$ make coverage_my
```

I have not yet figured out how to integrate it into a button click on VS code.


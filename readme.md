     / _____)             _              | |    
    ( (____  _____ ____ _| |_ _____  ____| |__  
     \____ \| ___ |    (_   _) ___ |/ ___)  _ \ 
     _____) ) ____| | | || |_| ____( (___| | | |
    (______/|_____)_|_|_| \__)_____)\____)_| |_|
        ©2013 Semtech

SX1272/76 radio drivers plus Ping-Pong firmware and LoRa MAC node firmware implementation.
=====================================

1. Introduction
----------------
The aim of this project is to show an example of the LoRaMac endpoint firmware
implementation.

**REMARK 1:** Currently only devices of Class A are supported.

**REMARK 2:** The SX1276 device is not yet supported.

2. System schematic and definitions
------------------------------------
The available supported hardware platforms schematics and LoRaMac specification
can be found in the Doc directory.

3. Dependencies
----------------

This program uses the AES algorithm implementation (http://www.gladman.me.uk/)
by Brian Gladman.

This program uses the CMAC algorithm implementation 
(http://www.cse.chalmers.se/research/group/dcs/masters/contikisec/) by 
Lander Casado, Philippas Tsigas.

This program depends on specific hardware platforms. Currently the supported 
platforms are:

    - Bleeper
        MCU     : STM32L151RD - 384K FLASH, 48K RAM, Timers, SPI, I2C,
                                USART, 
                                USB 2.0 full-speed device/host/OTG controller,
                                DAC, ADC, DMA
        RADIO   : SX1272
        ANTENNA : Connector for external antenna
        BUTTONS : 1 Reset, 16 position encoder
        LEDS    : 3
        SENSORS : Temperature
        GPS     : Possible through pin header GPS module connection
        SDCARD  : Yes
        EXTENSION HEADER : Yes, 12 pins
        REMARK  : None.

    - LoRaMote-eu868
        MCU     : STM32L151CB - 128K FLASH, 10K RAM, Timers, SPI, I2C,
                                USART, 
                                USB 2.0 full-speed device/host/OTG controller,
                                DAC, ADC, DMA
        RADIO   : SX1272
        ANTENNA : Printed circuit antenna
        BUTTONS : No
        LEDS    : 3
        SENSORS : Proximity, Magnetic, 3 axis Accelerometer, Pressure,
                  Temperature
        GPS     : Yes, UP501 module
        SDCARD  : No
        EXTENSION HEADER : Yes, 20 pins
        REMARK  : The MCU and Radio are on an IMST iM880A module

4. Usage
---------
Projects for Ride7 and Keil Integrated Development Environments are available.

5. Changelog
-------------
2014-02-21, v2.0

* General
    1. The LoRaMac applications now sends the LED status plus the sensors values.
       For the LoRaMote platform the application also sends the GPS coordinates.
       
        * The application payload for the Bleeper platform is as follows:
        
            LoRaMac port 1:
            
                 { 0x00/0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
                  ----------  ----------  ----------  ----------  ----
                       |           |           |           |        |
                      LED      PRESSURE   TEMPERATURE  ALTITUDE  BATTERY
                                                     (barometric)

        * The application payload for the LoRaMote platform is as follows:
        
            LoRaMac port 2:
            
                 { 0x00/0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
                  ----------  ----------  ----------  ----------  ----  ----------------  ----------------  ----------
                       |           |           |           |        |           |                 |              |
                      LED      PRESSURE   TEMPERATURE  ALTITUDE  BATTERY    LATITUDE          LONGITUDE      ALTITUDE 
                                                     (barometric)                                              (gps)
    2. Adapted applications to the new MAC layer API.
    3. Added sensors drivers implementation.
    4. Corrected new or known issues.
* LoRaMac
    1. MAC commands implemented
        * LinkCheckReq                 **YES**
        * LinkCheckAns                 **YES**
        * LinkADRReq                   **YES**
        * LinkADRAns                   **YES**
        * DevStatusReq                 **YES**
        * DevStatusAns                 **YES**
        * JoinReq                      **YES (Not tested yet)**
        * JoinAccept                   **YES (Not tested yet)**
    2. New MAC layer application API implementation.
* Timers and RTC.
    1. Still some issues. They will be corrected on next revisions of the firmware.

2014-01-24, v1.1

* LoRaMac
    1. MAC commands implemented
        * LinkCheckReq                 **NO**
        * LinkCheckAns                 **NO**
        * LinkADRReq                   **YES**
        * LinkADRAns                   **YES**
        * DevStatusReq                 **YES**
        * DevStatusAns                 **YES**
    2. Implemented an application LED control
        If the server sends on port 1 an application payload of one byte with
        the following contents:
        
            0: LED off
            1: LED on
        The node transmits periodically on port 1 the LED status on 1st byte and
        the message "Hello World!!!!" the array looks like:
        
            { 0, 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!' }
* Timers and RTC.
    1. Corrected issues existing in the previous version
    2. Known bugs are: 
        * There is an issue when launching an asynchronous Timer. Will be solved
          in a future version 

2014-01-20, v1.1.RC1

* Added Doc directory. The directory contains:
    1. LoRa MAC specification
    2. Bleeper board schematic
* LoRaMac has been updated according to Release1 of the specification. Main changes are:
    1. MAC API changed.
    2. Frame format.
    3 ClassA first ADR implementation.
    4. MAC commands implemented
        * LinkCheckReq              **NO**
        * LinkCheckAns              **NO**
        * LinkADRReq                **YES**
        * LinkADRAns                **NO**
        * DevStatusReq              **NO**
        * DevStatusAns              **NO**
    
* Timers and RTC rewriting. Known bugs are: 
    1. The Radio wakeup time is taken in account for all timings. 
    2. When opening the second reception window the microcontroller sometimes doesn't enter in low power mode.

2013-11-28, v1.0

* Initial version of the LoRa MAC node firmware implementation.

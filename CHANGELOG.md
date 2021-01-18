# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## LoRaWAN pre-certification results

Please refer to [Releases pre-certification-results](https://github.com/Lora-net/LoRaMac-node/wiki/releases-pre-certification-results) document for further information.

## [Unreleased]

## [4.5.1] - 2021-01-18


### General

- Release based on "LoRaWAN specification 1.0.4" and "LoRaWAN Regional Parameters 2-1.0.1"
- GitHub reported issues corrections.

### Fixed

- Fixed compiling issues

## [4.4.7] - 2021-01-18

### General

- Release based on "LoRaWAN specification 1.0.3" and "LoRaWAN Regional Parameters v1.0.3revA"
- **Release 4.4.7 is an hotfix of 4.4.6 and is the last one supporting "LoRaWAN specification 1.0.3"**

### Changed

- Changed examples GitHub version to 4.4.7

### Fixed

- Fixed compiling issues when KR920 or RU864 regions are selected
- Fixed compiling issues for `fuota-test-01` example


## [4.5.0] - 2020-12-18

### General

- Release based on "LoRaWAN specification 1.0.4" and "LoRaWAN Regional Parameters 2-1.0.1"
- GitHub reported issues corrections. Please refer to [Release Version 4.5.0](https://github.com/Lora-net/LoRaMac-node/milestone/5)

### Added

- Added implementation of LoRaWAN 1.0.4 changes
- Added implementation for new Certification protocol specification
- Added GitHub repository and firmware examples version definitions

## [4.4.6] - 2020-12-18

### General

- Release based on "LoRaWAN specification 1.0.3" and "LoRaWAN Regional Parameters v1.0.3revA"
- GitHub reported issues corrections. Please refer to [Release Version 4.4.6](https://github.com/Lora-net/LoRaMac-node/milestone/8)
- **Release 4.4.6 is the last one supporting "LoRaWAN specification 1.0.3"**

### Changed

- Refactored MAC layer and examples Non Volatile Memory data handling
- Changed `NewChannelReq` amd `DlChannelReq` handling to ignore command for `AU915`, `CN470` and `US915` regions
- Updated behavior of `SX126xSetLoRaSymbNumTimeout` to round up the timeout to the nearest possible number of symbol.
- Changed radio drivers floating point to integer computations
- Improved `GetNextLowerTxDr` function implementation.
- Changed SX1272 and SX1276 FSK FIFO handling by using `FifoLevel` IRQ instead of `FifoEmpty` IRQ  
  **WARNING** *DIO1 IRQ MCU pin detection has changed from `RISING` to `RISING_FALLING` edge.*
- Changed IMST boards default power source to `USB_POWER`

### Fixed

- Fixed certification test application handling
- Fixed `ping-pong` and `rx-sensi` examples to use the right `FSK_BANDWIDTH` and `FSK_AFC_BANDWIDTH` parameters
- Fixed one byte transmissions over I2C
- Fixed `RegionIN865RxParamSetupReq` `DrOffset` verification
- Fixed STM platforms IRQ handlers when using a C++ compiler
- Fixed an issue where the Join back off algorithm wasn't being applied when the duty-cycle enforcement was OFF
- Fixed `AU915_MAX_RX1_DR_OFFSET` value to 5 instead of 6

### Removed

- Removed ClassA, ClassB and ClassC examples.

## [4.5.0-rc.1] - 2020-11-24

### General

- Release based on "LoRaWAN specification 1.0.4" and "LoRaWAN Regional Parameters 2-1.0.1"
- GitHub reported issues corrections. Please refer to [Release Version 4.5.0](https://github.com/Lora-net/LoRaMac-node/milestone/5)

### Added

- Added implementation of LoRaWAN 1.0.4 changes
- Added implementation for new Certification protocol specification
- Added GitHub repository and firmware examples version definitions

### Changed

- Changed IMST boards default power source to USB_POWER

### Fixed

- Fixed an issue where the Join back off algorithm wasn't being applied when the duty-cycle enforcement was OFF.
- Fixed `AU915_MAX_RX1_DR_OFFSET` value to 5 instead of 6

### Removed

- Removed NvmCtxCallback calls as they should only be done when a modification happens.
- Removed ClassA, ClassB and ClassC examples.

## [4.4.5] - 2020-10-14

### General

- Release based on "LoRaWAN specification 1.0.3" and "LoRaWAN Regional Parameters v1.0.3revA"
- GitHub reported issues corrections. Please refer to [Release Version 4.4.5](https://github.com/Lora-net/LoRaMac-node/milestone/7)

### Added

- Added to `FRAGMENTATION_FRAG_SESSION_SETUP_REQ` checks which verify `FragNb` and `FragSize` validity.
- Added missing call to `SX126xSetOperatingMode` on `SX126xSetTxContinuousWave` and `SX126xSetTxInfinitePreamble` functions
- Added new specific board API to initialize the RF switch (`SX126xIoRfSwitchInit`)
- Add to AS923 and KR920 regions a definition for the Rx bandwidth to be used while executing the LBT algorithm
- Added support for other AS923 channel sub plan groups.
- Added FPort filtering to loramac handler packages.
- Added missing NVM update notifications

### Changed

- Refactored function `RegionXXInitDefaults`. Fixes an issue on US based bands where the channels mask was not being reset.
- Changed hard coded `JoinAccept` max payload size (33) by `LORAMAC_JOIN_ACCEPT_FRAME_MAX_SIZE` definition.
- Moved radio operating mode management to specific board implementation
- Changed radio `IsChannelFree API` in order to provide reception bandwidth
- AS923 default downlink dwell time setting changed to disabled
- Back port 1.0.4 region implementation improvements
- Changed `RegionCommonComputeSymbolTimeLoRa`, `RegionCommonComputeSymbolTimeFsk` and `RegionCommonComputeRxWindowParameters`
  API implementations to use integer divisions instead of double division.
- Changed DeriveSessionKey10x and DeriveSessionKey11x functions API (Removed pointer based variable usage)

### Fixed

- Fixed `SX126xSetLoRaSymbNumTimeout` to call the workaround only if the number of symbols is equal to or higher than 64.
- Fixed IAR C++ compiler warnings
- Fixed missing `{` after `extern "C"` in `cmac.h` file
- Fixed `FragSessionStatusAns` message construction for `Received&index` field
- Fixed `LoRaMacCryptoHandleJoinAccept` function `DevNonce`, `RJCount0` and `RJCount1` handling.
- Fixed nonce variable declaration
- Fixed `LoRaMacCrypto.c` conditional pre-processing.
- Fixed missing `Rx1Frequency` reset for dynamic channel plans
- Applied Japan ARIB restrictions to the `AS923_1_JP` sub plan
- Applied Regional Parameters 1.0.3.revA ERRATA note concerning the ClassB beacon format
- Fixed SAMR34 compiling issue when `USE_RADIO_DEBUG` directive is defined
- Fixed `SX126xClearIrqStatus` call to only clear read flags
- Fixed GFSK bandwidth handling for SX126x and LR1110 radios.
- Fixed version handling for MC root keys derivation
- Fixed Class B initialization.
- Fixed duty-cycle implementation

## [4.4.4] - 2020-05-26

### General

- Release based on "LoRaWAN specification 1.0.3" and "LoRaWAN Regional Parameters v1.0.3revA"
- GitHub reported issues corrections. Please refer to [Release Version 4.4.4](https://github.com/Lora-net/LoRaMac-node/milestone/6)

### Added

- Added the possibility to setup multicast channels locally
- Added new `SecureElementProcessJoinAccept` API to the `secure-element.h`
- Added possibility to select the secure-element to be used as well as to select if it is pre-provisioned or not
- Added support for LR1110 radio
- Added support for LR1110 crypto-engine to be used as secure-element
- Added support for ATECC608A-TNGLORA secure-element
- Added `LoRaMacDeInitialization` API to ease dynamic region change
- Added possibility to query implemented versions of the LoRaWAN MAC layer and Regional Parameters specifications
- Added `MOTE_MAC_TX_PARAM_SETUP_ANS` as sticky MAC command
- Added a filter to handle multicast downlinks exceptions
- Added `CHANGELOG.md` file

### Changed

- Updated examples to use OTAA activation method by default
- Updated `LmhpFragmentation` implementation
- Updated Radio APIs to handle 32-bit register addresses
- Changed SX126x driver to only process a received packet when there is no CRC error
- Moved `Commissioning.h` files to a single one located under `src/apps/LoRaMac/common` directory
- Updated `Radio.TimeOnAir` API in order to be independent of the current hardware settings
- Moved keys definition and management to the secure-element implementation
- Moved device identity to secure-element `se-identity.h` file
- Changed support of SAML21 platform by SAMR34 platform
- Changed the `MlmeRequest` and `McpsRequest` APIs to report back when the next transmission is possible
- Changed `ResetMacParameters` to perform a complete re-initialization of the regional layer after processing the join accept
- Updated the implementation to verify the MAC commands size
- Changed the way the duty-cycle management is handled

### Removed

- Removed the LoRaWAN keys display on serial terminal
- Removed `GEN_APP_KEY` support as it is equivalent to LoRaWAN 1.1.x `APP_KEY`
- Removed tables definition for repeater support. LoRa-Alliance will provide details on these tables usage in future versions
- Removed Changelog chapter from `readme.md` file

### Fixed

- Fixed `SX126xGetRandom` API implementation
- Fixed SX126x driver missing call to `RadioStandby` in LoRa modem case inside `RadioSetRxConfig` API
- Fixed SX126x driver `LoRaSymbNumTimeout` handling
- Fixed soft-se/cmac for misaligned 32-bit access
- Fixed ping-pong and rx-sensi projects for packets with sizes bigger than 64 bytes handling
- Fixed missing switch case `PHY_BEACON_CHANNEL_FREQ` for *AU915* region
- Fixed default data rates for Class B beacon and ping-slots for *AU915* region
- Fixed join request limitation due to duty cycle restrictions
- Fixed `SRV_MAC_DEVICE_TIME_ANS`, `SRV_MAC_PING_SLOT_INFO_ANS` and `SRV_MAC_BEACON_TIMING_ANS` processing to be executed only if corresponding MLME request is queued
- Fixed issue where the stack could get stuck in MCPS busy state after MLME Device Time Request
- Fixed duty-cycle enforcement by disabling it for *IN865* region
- Fixed potential issue where the `FCntUp` counter could sometimes be incremented by 2 instead of 1
- Fixed wrong beacon time-on-air calculation

### Security

- Security breach found by Tencent Blade Team please refer to [security advisory - CVE-2020-11068](https://github.com/Lora-net/LoRaMac-node/security/advisories/GHSA-559p-6xgm-fpv9)

## [4.4.3] - 2019-12-17

### General

- Release based on "LoRaWAN specification 1.0.3" and "LoRaWAN Regional Parameters v1.0.3revA"
- GitHub reported issues corrections. Please refer to [Release Version 4.4.3](https://github.com/Lora-net/LoRaMac-node/milestone/4)

## [4.4.2] - 2019-07-19

### General

- Release based on "LoRaWAN specification 1.0.3" and "LoRaWAN Regional Parameters v1.0.3revA"
- GitHub reported issues corrections. Please refer to [Release Version 4.4.2](https://github.com/Lora-net/LoRaMac-node/milestone/2)

### Added

- Added new refactored application examples
- Added LoRa-Alliance defined application layer protocols support. Only FUOTA test scenario 01 required features are currently implemented  
  - "Clock Synchronization" package
  - "Fragmented data block transport" package
  - "Remote multicast setup" package
- Added a NVM context management module. (Disabled by default)
- Added secure-element support
- Added B-L072Z-LRWAN1 platform support
- Added NucleoL476 platform support
- Added NucleoL476 platform support
- Added IMST new platforms
- Added *RU864* region support
- Added ClassB support
- Added a callback to notify the upper layer to call `LoRaMacProcess` function
- Added support for RxC windows (ClassC) required by the application layer protocols

### Changed

- Examples application refactoring plus the addition of the application status display on the serial port. (921600-8-N-1)
- Refactored `timer.c/h`, `rtc-driver.c/h` and added a `systime.c/h` module
- Functions in ISR context have been moved to main context
- Changed `BoardDisableIrq` and `BoardEnableIrq` functions by `CRITICAL_SECTION_BEGIN` and `CRITICAL_SECTION_END`
  respectively
- Heavily refactored the `LoRaMac.c` and `LoRaMac.h` implementation
- Changed multicast channels handling according to the application layer protocols

### Removed

- Removed SensorNode platform support
- Removed MoteII platform support
- Removed LoRaMote platform support
- Removed *US915-Hybrid* region support

## Fixed

- Applied SX1272 and SX1276 radios errata note 3.1 to the radio drivers implementation
- Fixed `printf` and `scanf` functions when GCC is used as compiler

## [4.4.1] - 2018-03-07

### General

- Release based on "LoRaWAN specification 1.0.2" and "LoRaWAN Regional Parameters v1.0.2rB"
- GitHub reported issues corrections. Please refer to [Release Version 4.4.1](https://github.com/Lora-net/LoRaMac-node/milestone/1)

### Added

- Added SX126x radio support
- Added NucleoL073 and NucleoL152 board platforms support
- Added Microchip/Atmel SAML21 Xplained Pro hardware platform support
- Added CMAKE build system support

### Removed

- Removed CoIDE projects support
- Removed Keil projects support

## [4.4.0] - 2017-09-08

### General

- Release based on "LoRaWAN specification 1.0.2" and "LoRaWAN Regional Parameters v1.0.2rB"
- GitHub reported issues corrections

### Added

- Added SX126x radio support
- Added NucleoL073 and NucleoL152 board platforms support
- Added Microchip/Atmel SAML21 Xplained Pro hardware platform support
- Added CMAKE build system support
- Added the support for all "LoRaWAN Regional Parameters v1.0.2rB" document defined regions

### Changed

- Changed radio driver to perform the LBT carrier sense continuously for a given period of time
- Changed implementation to support 1.0.2 specification additions

## [4.3.2] - 2017-04-19

### General

- Release based on "LoRaWAN specification 1.0.2"
- GitHub reported issues corrections

### Added

- Added an algorithm to automatically compute the Rx windows parameters. (Window `symbolTimeout` and `Offset` from
 downlink expected time)
- Added a workaround to reset the radio in case a `TxTimeout` occurs
- Added newly defined `TxCw`(Tx Continuous Wave) certification protocol command

### Changed

- Changed radio driver to perform the LBT carrier sense continuously for a given period of time
- Changed implementation to support 1.0.2 specification additions
- Changed FSK modem handling to use the provided `symbolTimeout` (1 symbol equals 1 byte) when in `RxSingle` mode
- Changed `GpioMcuInit` function to first configure the output pin state before activating the pin
- Changed the `AdrAckCounter` handling as expected by the test houses
- Relocate parameter settings from `ResetMacParameters` to the initialization
- Changed MAC layer indications handling

### Removed

- Removed useless `LoRaMacPayload` buffer

### Fixed

- Fixed an overflow issue that could happen with `NmeaStringSize` variable
- Fixed an issue where the node stopped transmitting


## [4.3.1] - 2017-02-27

### General

- Release based on "LoRaWAN specification 1.0.1"
- GitHub reported issues corrections

### Added

- Added MoteII platform based on the IMST module iM881A (STM32L051C8)
- Added NAMote72 platform
- Added TxCw (Tx continuous wave) LoRaWAN compliance protocol command
- Added TxContinuousWave support to the radio drivers
- Added region CN470 support

### Changed

- Updated the MAC layer in order to be LoRaWAN version 1.0.1 compliant (Mainly US915 bug fixes)
- Updated SensorNode projects according to the new MCU reference STM32L151CBU6. Bigger memories
- Updated ST HAL drivers
  - STM32L1xx_HAL-Driver : V1.2.0
  - STM32L0xx_HAL_Driver : V1.7.0

### Removed

- Removed api-v3 support from the project

### Fixed

- Fixed compliance test protocol command 0x06 behavior
- Fixed US band in order to pass the LoRaWAN certification

## [4.3.0] - 2016-06-22

### General

- Release based on "LoRaWAN specification 1.0.1"
- GitHub reported issues corrections

### Added

- Added radio `RX_TIMEOUT` irq clear into the irq handler
- Added retransmission back-off
- Added debug pin support to LoRaMote platform
- Added a function to verify the RX frequency of window 2 for US band

### Changed

- Applied to all application files the certification protocol change for LoRaWAN 1.0.1 compliance tests  
  **REMARK**: api-v3 application files aren't updated
- Update of the STM32L0 HAL to version 1.6.0
- Consolidated the line endings across all project files
  Windows line endings has been choose for almost every file
- Updated maximum payload size for US band
- Update data rate offset table for US band
- Make MAC commands sticky
- Updated and improved MPL3115 device driver
- RTC driver major update

### Removed

- Removed the end less loop from `HAL_UART_ErrorCallback`
- Remove the `TxPower` limitation for US band on `LoRaMacMibSetRequestConfirm` function. The power will be limited
  anyway when the `SendFrameOnChannel` functions is called

### Fixed

- Issue(#81): Bug fix in function `LoRaMacMlmeRequest` case `MLME_JOIN`. Function will return `LORAMAC_STATUS_BUSY` in
  case the MAC is in status `MAC_TX_DELAYED`
- Issue(#83): Bug fix in parameter validation
- Issue(#84): Fix issue of `CalibrateTimer` function
- Applied pull request #87
- Issue(#88): Bug fix in function `PrepareFrame` where repeated MAC commands were not handled correctly
- Fixed `OnRadioRxDone` Node now drops frames on port 0 with fOpts > 0
- Fixed `OnRadioRxDone` Node now receives frames with fOpts > 0 when no payload is present

## [4.2.0] - 2016-05-13

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections

### Added

- Added a definition for the LoRaWAN device address. Add an IEEE_OUI for the LoRaWAN device EUI
- Added a definition for the default data rate
- Introduce individual MIN and MAX datarates for RX and TX
- Added the possibility to set and get the `ChannelsDefaultDatarate`
- Introduce a new status `MAC_RX_ABORT`. Reset `MAC_TX_RUNNING` only in `OnMacStateCheckTimerEvent`

### Changed

- Update STM32L1xx_HAL_Driver version to 1.5. Update related drivers and implementations accordingly  
  **REMARK**: This change implies that the time base had to be changed from microseconds to milliseconds
- Optimize radio drivers regarding FSK PER
- Increase RTC tick frequency for higher resolution
- Update the radio wake up time
- Update channel mask handling for US915 hybrid mode to support any block in the channel mask
- Optimization of the RX symbol timeout
- Replace definition `LORAMAC_DEFAULT_DATARATE` by `ChannelsDefaultDatarate` in `LoRaMacChannelAdd`
- Accept MAC information of duplicated, confirmed downlinks

### Fixed

- Fixed the frequency check condition for // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
- Fixed an issue when calling `SX127xInit` function more than once
- Issue(#66) correction of functions `SX1276SetOpMode` and `SX1272SetOpMode`
- Issue(#68): Fix for low level RF switch control
- Issue(#56) correction
- Issue(#63) correct the maximum payload length in RX mode
- Fixed Tx power setting loss for repeated join requests on US band
- Issue(#59): Add the possibility to set the uplink and downlink counter
- Issue(#72): Fix of possible array overrun in `LoRaMacChannelRemove`
- Issue(#74): Drop frames with a downlink counter difference greater or equal to `MAX_FCNT_GAP`

## [4.1.0] - 2016-03-10

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections
- This version has passed all mandatory LoRa-Alliance compliance tests  
  *One of the optional tests is unsuccessful (FSK downlinks PER on Rx1 and Rx2 windows) and is currently under investigation*

### Added

- Add missing documentation about `MIB_REPEATER_SUPPORT`. (Issue #42)
- Add a new `LoRaMacState` to allow adding channels during TX procedure. (Issue #43)
- Add a new function `PrepareRxDoneAbort` to prepare a break-out of `OnRadioRxDone` in case of an error

### Changed

- Application state machine. Relocate setting sleep state and update the duty cycle in compliance test mode
- Changed `TimerHwDelayMs` function to be re-entrant
- Rename attribute `nbRetries` to `NbTrials` in structure `McpsReqConfirmed_t`. (Issue #37)
- Updated implementation of `SetNextChannel`. Added enabling default channels in case of join request. (Issue #39)
- Relocate the activation of `LoRaMacFlags.Bits.McpsInd` in `OnRadioRxDone`
- Reset status of `NodeAckRequested` if we received an ACK or in case of timeout
- Improved and corrected `AdrNextDr` function

### Removed

- Removed support for Raisonance Ride7 IDE (Reduces the amount of work to be done at each new release)
- Removed the Bleeper-72 and Bleeper-76 platforms support as these are now deprecated
- Removed additional EU868 channels from the LoRaWAN implementation files. GitHub (Issue #49)  
  The creation of these additional channels has been moved to the application example

### Fixed

- Fixed `TimerIrqHandler`. Now, it is possible to insert timers in callback
- Fixed FSK modem packets bigger than 64 bytes handling (Issue #36)
- Fixed default channels activation in case all others are disabled. (Issue #39)
- Fixed default channel setting in case none is enabled
- Fixed `SRV_MAC_NEW_CHANNEL_REQ` MAC command by adding a fix to the `macIndex` variable on US915 band
- Start the MacStateCheckTimer in OnRxDone and related error cases with a short interval to handle events promptly. (Issue #44)

## [4.0.0] - 2015-12-18

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections

### Added

- LoRaWAN certification protocol implementation

### Changed

- STACKFORCE new API integration
- Reverse the EUIs arrays in the MAC layer

## [3.4.1] - 2015-11-06

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections

### Fixed

- Fixed downlink counter roll over management when several downlinks were missed
- Fixed the Radio maximum payload length management. Radio was filtering received frames with a length bigger than the transmitted one
- Applied Pull request #22 solution proposition

## [3.4.0] - 2015-10-30

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections

### Added

- Added TimerStop function calls to each timer event callback
- Added some missing variables casts. Helps on code portability
- Added const variable attribute when necessary to functions prototypes
- Added `LORAMAC_MIN_RX1_DR_OFFSET` and `LORAMAC_MAX_RX1_DR_OFFSET` definitions to `LoRaMac-board.h`. Can be different
 upon used PHY layer
- Added the limitation of output power according to the number of enabled channels for US915 band
- Added the limitation of the applicative payload length according to the data rate. Does not yet take in account the
 MAC commands buffer. (Issue #15)

### Changed

- Changed all applications in order to have preprocessing definitions on top of the files and added relevant comments
- Applications LED control is no more done into the timer callback functions but instead on the main while loop
- Changed types names for `stdint.h` names. Helps on code portability
- Renamed `rand` and `srand` to `rand1` and `srand1`. Helps on code portability
- Moved `ID1`, `ID2` and `ID3` definition from `board.h` to `board.c`, `usb-cdc-board.c` and `usb-dfu-board.c`
- Renamed `BoardMeasureBatterieLevel` to `BoardGetBatteryLevel`
- Added `SetMaxPayloadLength` API function to SX1272 and SX1276 radio drivers
- Changed the name of Radio API Status function to `GetStatus`
- Changed AES/CMAC types names for `stdint.h` names. Helps on code portability (Issue #20)
- Moved `__ffs` function from `utilities.h` to `spi-board.c`. This function is only used there
- Added support for the US915 band (Normal mode and hybrid mode. Hybrid mode is a temporary configuration up until servers support it automatically) (Issue #16)
- Replaced the usage of `__IO` attribute by `volatile`
- Renamed `LoRaMacEvent_t` into `LoRaMacCallbacks_t` and added a function pointer for getting battery level status
- Renamed `LoRaMacSetDutyCycleOn` into `LoRaMacSetTestDutyCycleOn`
- Renamed `LoRaMacSetMicTest` into `LoRaMacTestSetMic`
- Increased the PHY buffer size to 250
- `LoRaMacEventInfo.TxDatarate` now returns LoRaWAN data rate (DR0 -> DR7) instead of (SF12 -> DF7)
- `LoRaMac-board.h` now implements the settings for the PHY layers specified by LoRaWAN 1.0 specification. (EU433, CN780, EU868, US915) (Issue #19)

### Removed

- Removed `NULL` definition from `board.h`
- Removed the definition of `RAND_SEED`. It has been replaced by a function named `BoardGetRandomSeed`
- Removed `fputc` function redefinition from `Utilities.c`
- Removed the usage of `PACKED` attribute for data structures
- Removed `IsChannelFree` check on `LoRaMacSetNextChannel` function. LoRaWAN is an ALHOA protocol. (Pull request #8)

### Fixed

- Fixed timings comments. Timing values are most of the time us based
- Fixed and simplified the downlink sequence counter management
- Fixed channel mask management for EU868 band
- Fixed `LoRaMacPrepareFrame` function behavior when no applicative payload is present
- Fixed `MacCommandBufferIndex` management. (Issue #18)

## [3.3.0] - 2015-08-07

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections

### Added

- Added the support for LoRaWAN Class C devices
- Added the possibility to change the channel mask and number of repetitions trough SRV_MAC_LINK_ADR_REQ command when ADR is disabled

### Changed

- Increased FSK SyncWord timeout value in order to listen for longer time if a down link is available or not. Makes FSK downlink more reliable
- Increased the UART USB FIFO buffer size in order to handle bigger chunks of data
- Renamed data rates as per LoRaWAN specification
- Changed confirmed messages function to use default datarate when ADR control is off
- After a Join accept the node falls back to the default datarate. Enables the user to Join a network using a different datarate from its own default one
- Moved the MAC commands processing to the right places. FOpts field before the Payload and Port 0 just after the decryption. (Pull request #9)

### Fixed

- Implemented the radios errata note workarounds. SX1276 errata 2.3 "Receiver Spurious Reception of a LoRa Signal" is not yet implemented
- Fixed handling of the MAC commands which was done incorrectly the condition to verify the length of the buffer has changed from `<` to `<=`
- Fixed `Rx1DrOffset` management. In previous version DR1 was missing for all offsets
- Fixed default FSK channel frequency
- Fixed a firmware freeze when one of the following situations arrived in `OnRxDone` callback: bad address, bad MIC, bad frame. (Pull request #10)
- Fixed weird conditions to check data rate on MAC command `SRV_MAC_NEW_CHANNEL_REQ` (Pull request #7)
- Fixed join accept message handling to be ignored if already joined (Pull request #6)
- Fixed channel index verification should use OR on `SRV_MAC_NEW_CHANNEL_REQ` command (Pull request #5)
- Fixed the CFList management on JoinAccept. The `for` loop indexes were wrong. (Pull request #4)
- Fixed AES key size (Pull request #3)

## [3.2.0] - 2015-04-30

### General

- Release based on "LoRaWAN specification 1.0.0"
- GitHub reported issues corrections

### Added

- Added the support of packed structures when using IAR tool chain
- Added a function to get the time in us
- Added a typedef for time variables (TimerTime_t)
- Added a function that returns if the GPS has a fix or not
- Added missing MAC commands and updated others when necessary
  - Corrected the Port 0 MAC commands decryption
  - Changed the way the upper layer is notified. Now it is only  notified
    when all the operations are finished

      When a ClassA Tx cycle starts a timer is launched to check every second if everything is finished

  - Added a new parameter to `LoRaMacEventFlags` structure that indicates on which Rx window the data has been received
  - Added a new parameter to `LoRaMacEventFlags` structure that indicates if there is applicative data on the received payload
  - Corrected ADR MAC command behavior
  - DutyCycle enforcement implementation (EU868 PHY only)

    **REMARK 1** *The regulatory duty cycle enforcement is enabled by default which means that for lower data rates the
     node may not transmit a new frame as quickly as requested  
    The formula used to compute the node idle time is*

        Toff = TimeOnAir / DutyCycle - TxTimeOnAir

    *Example:*  
    *A device just transmitted a 0.5 s long frame on one default channel
    This channel is in a sub-band allowing 1% duty-cycle. Therefore this whole sub-band (868 MHz - 868.6 MHz) will be
     unavailable for 49.5 s.*

    **REMARK 2** *The duty cycle enforcement can be disabled for test purposes by calling the `LoRaMacSetDutyCycleOn`
     function with false parameter.*
  - Implemented aggregated duty cycle management
  - Added a function to create new channels
  - Implemented the missing features on the JoinAccept MAC command
  - Updated LoRaMacJoinDecrypt function to handle the CFList field
- Added the possibility to chose to use either public or private networks

### Changed

- Cosmetic updates
- Changed the TimeOnAir radio function to return a uint32_t value instead of a double. The value is in us
- Changed the GetPosition functions to return a latitude and longitude of 0 and altitude of 65535 when no GPS fix
- Due to duty cycle management the applicative API has changed  
  All applications must be updated accordingly

### Removed

- Removed support for previous LoRaMac/LoRaWAN specifications

### Fixed

- Fixed the 250 kHz bandwidth choice for the FSK modem

## [3.1.0] - 2015-01-30

### General

- Release based on "LoRaMac specification R3.0.0"
- GitHub reported issues corrections

### Added

- Started to add support for CooCox CoIDE Integrated Development Environment  
  Currently only LoRaMote and SensorNode platform projects are available
- Added the support of different tool chains for the `HardFault_Handler` function
- Added an invalid bandwidth to the Bandwidths table in order to avoid an error
 when selecting 250 kHz bandwidth when using FSK modem
- Added another timer in order increment the tick counter without blocking the normal timer count
- Added the possibility to switch between low power timers and normal timers on the fly
- Added an I2C EEPROM driver
- Added the possibility to analyze an hard fault interrupt
- Added the possibility to configure almost all the MAC parameters
- Added the functions to read the Up and Down Link sequence counters

### Changed

- Updated GCC compiler linker scripts
- Changed the RSSI values type from int8_t to int16_t. We can have RSSI values below -128 dBm
- Updated radio API to support FHSS and CAD handling
- Changed all structures declarations to be packed
- Automatically activate the channel once it is created

### Fixed

- Fixed Radio drivers I&Q signals inversion to be possible in Rx and in Tx  
  Added some missing radio state machine initialization
- Fixed SNR computation on RxDone interrupt
- Fixed in `SetRxConfig` function the FSK modem preamble register name
- Fixed RTC alarm setup which could be set to an invalid date
- Fixed I2C driver by correcting the 2 bytes internal address management  
  Corrected buffer read function when more that 1 byte was to be read
  Added a function to wait for the I2C bus to become IDLE
- Fixed and improved USB Virtual COM Port management files  
  Corrected the USB CDC and USB UART drivers
- Fixed RxWindow2 data rate management
- Fixed `SrvAckRequested` variable was never reset
- Fixed tstIndoor applications for LoRaMac R3.0 support
- Fixed the `LoRaMacSetNextChannel` function
- Fixed the port 0 MAC command decoding
- Fixed the Acknowledgment retries management when only 1 trial is needed  
  Before the device was issuing at least 2 trials
- Fixed server mac new channel req answer
- Fixed `SRV_MAC_RX2_SETUP_REQ` frequency handling
  Added a x100 multiplication
- Fixed `SRV_MAC_NEW_CHANNEL_REQ`  
  Removed the DutyCycle parameter decoding
- Fixed `NbRepTimeoutTimer` initial value. `RxWindow2Delay` already contains `RxWindow1Delay` in it

## [3.0.0] - 2014-07-18

### General

- Release based on "LoRaMac specification R3.0.0"
- GitHub reported issues corrections

### Added

- Added to Radio API the possibility to select the modem

### Changed

- Changed radio callbacks events to timeout when it is a timeout event and error when it is a CRC error
- Radio API updated
- Updated ping-pong applications
- Updated tx-cw applications
- Updated LoRaMac applications in order to handle LoRaMac returned functions calls status
- Updated LoRaMac applications to toggle LED2 each time there is an application payload down link
- Updated tstIndoor application to handle correctly more than 6 channels
- Changed the MPL3115 altitude variable from unsigned to signed value
- Changed the usage of pow(2, n) by defining POW2 functions Saves ~2 KBytes of code
- MAC commands implemented
  - LinkCheckReq                        **YES**
  - LinkCheckAns                        **YES**
  - LinkADRReq                          **YES**
  - LinkADRAns                          **YES**
  - DutyCycleReq                        **YES**
  - DutyCycleAns                        **YES**
  - Rx2SetupReq                         **YES**
  - Rx2SetupAns                         **YES**
  - DevStatusReq                        **YES**
  - DevStatusAns                        **YES**
  - JoinReq                             **YES**
  - JoinAccept                          **YES**
  - NewChannelReq                       **YES**
  - NewChannelAns                       **YES**
- Features implemented
  - Possibility to shut-down the device **YES** (Possible by issuing DutyCycleReq MAC command)
  - Duty cycle management enforcement   **NO**
  - Acknowledgments retries             **YES**
  - Unconfirmed messages retries        **YES**

### Fixed

- Fixed RSSI reading formulas as well as changed the RSSI and SNR values from double to int8_t type
- Fixed an issue potentially arriving when LOW_POWER_MODE_ENABLE wasn't defined  
  A timer interrupt could be generated while the TimerList could already be emptied

## [2.3.0-RC2] - 2014-07-10

### General

- Release based on "LoRaMac specification R2.2.1"
- GitHub reported issues corrections

### Fixed

- Fixed all radios antenna switch low power mode handling
- Fixed SX1276 antenna switch control

## [2.3.0-RC1] - 2014-06-06

### General

- GitHub reported issues corrections

### Added

- Added the support for SX1276 radio
- Added Bleeper-76 and SensorNode platforms support
- Added to the radio drivers a function that generates a random value from RSSI readings
- Added a project to transmit a continuous wave and a project to measure the the radio sensitivity
- Added a bootloader project for the LoRaMote and SensorNode platforms
- Added a function to the timer driver that checks if a timer is already in the list or not
- Added the UART Overrun bit exception handling to the UART driver

### Changed

- Radio driver RxDone callback function API has changed (size parameter is no more a pointer)
  - Previous function prototype:  
    `void    ( *RxDone )( uint8_t *payload, uint16_t *size, double rssi, double snr, uint8_t rawSnr );`
  - New function prototype:  
    `void    ( *RxDone )( uint8_t *payload, uint16_t size, double rssi, double snr, uint8_t rawSnr );`
- The LoRaMac application for Bleeper platforms now sends the Selector and LED status plus the sensors values
  - The application payload for the Bleeper platforms is as follows:  
    LoRaMac port 1:

        { 0xX0/0xX1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
          ----------  ----------  ----------  ----------  ----
              |           |           |           |        |
        SELECTOR/LED  PRESSURE   TEMPERATURE  ALTITUDE  BATTERY
        MSB nibble = SELECTOR               (barometric)
        LSB bit    = LED
- Redefined `rand()` and `srand()` standard C functions. These functions are redefined in order to get the same behavior across different compiler tool chains implementations
- GPS driver improvements. Made independent of the board platform
- Simplified the RTC management
- MAC commands implemented
  - LinkCheckReq                        **YES**
  - LinkCheckAns                        **YES**
  - LinkADRReq                          **YES**
  - LinkADRAns                          **YES**
  - DutyCycleReq                        **YES** (LoRaMac specification R2.2.1)
  - DutyCycleAns                        **YES** (LoRaMac specification R2.2.1)
  - Rx2SetupReq                         **YES** (LoRaMac specification R2.2.1)
  - Rx2SetupAns                         **YES** (LoRaMac specification R2.2.1)
  - DevStatusReq                        **YES**
  - DevStatusAns                        **YES**
  - JoinReq                             **YES**
  - JoinAccept                          **YES** (LoRaMac specification R2.2.1)
  - NewChannelReq                       **YES** (LoRaMac specification R2.2.1)
  - NewChannelAns                       **YES** (LoRaMac specification R2.2.1)
- Features implemented
  - Possibility to shut-down the device **YES** (Possible by issuing DutyCycleReq MAC command)
  - Duty cycle management enforcement   **NO**
  - Acknowledgments retries             **WORK IN PROGRESS** (Not fully debugged. Disabled by default)
  - Unconfirmed messages retries        **WORK IN PROGRESS** (LoRaMac specification R2.2.1)
- Due to new specification the `LoRaMacInitNwkIds` LoRaMac API function had to be modified
  - Previous function prototype:  
      `void LoRaMacInitNwkIds( uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey );`
  - New function prototype:  
      `void LoRaMacInitNwkIds( uint32_t netID, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey );`
- Changed the LoRaMac channels management
- LoRaMac channels definition has been moved to `LoRaMac-board.h` file located in each specific board directory

### Removed

- Removed dependency of spi-board files to the `__builtin_ffs` function  
    This function is only available on GNU compiler tool suite  
    Removed --gnu compiler option from Keil projects  
    Added own `__ffs` function implementation to `utilities.h` file
- Removed obsolete class1 devices support

### Fixed

- Fixed Radio continuous reception mode

## [2.2.0] - 2014-04-07

### General

- GitHub reported issues corrections

### Added

- Added IMST SK-iM880A starter kit board support to the project
  - The application payload for the SK-iM880A platform is as follows:  
    LoRaMac port 3:

            { 0x00/0x01, 0x00, 0x00, 0x00 }
             ----------  ----- ----------
                  |        |       |
                 LED     POTI     VDD

### Changed

- Ping-Pong applications have been split per supported board
- Made fifo functions coding style coherent with the project
- UART driver is now independent of the used MCU

### Fixed

- Fixed the SX1272 output power management
  - Added a variable to store the current Radio channel
  - Added missing FSK bit definition

## [2.1.0] - 2014-03-28

### General

- GitHub reported issues corrections

### Added

- Added to the radio driver the possibility to choose the preamble length and rxSingle symbol timeout in reception
- Added Hex coder selector driver for the Bleeper board
- Added acknowledgments retries management
  - Split the `LoRaMacSendOnChannel` function in `LoRaMacPrepareFrame` and `LoRaMacSendFrameOnChannel`
     `LoRaMacSendOnChannel` now calls the 2 newly defined functions  
    **WARNING**: By default the acknowledgment retries specific code isn't enabled  
    The current http://iot.semtech.com server version doesn't support it
- Added `LORAMAC_EVENT_INFO_STATUS_MAC_ERROR` event info status

### Changed

- The timers and RTC management has been rewritten
- Improved the UART and UP501 GPS drivers
- Changed copyright Unicode character to (C) in all source files
- MAC commands implemented
  - LinkCheckReq                 **YES**
  - LinkCheckAns                 **YES**
  - LinkADRReq                   **YES**
  - LinkADRAns                   **YES**
  - DevStatusReq                 **YES**
  - DevStatusAns                 **YES**
  - JoinReq                      **YES**
  - JoinAccept                   **YES**

### Fixed

- Fixed GPIO pin names management
- Fixed the antenna switch management in the SX1272 driver
- Fixed issues on JoinRequest and JoinAccept MAC commands

## [2.0.0] - 2014-02-21

### General

- GitHub reported issues corrections

### Added

- Added new MAC layer application API implementation
- Added sensors drivers implementation

### Changed

- The LoRaMac applications now sends the LED status plus the sensors values
  For the LoRaMote platform the application also sends the GPS coordinates
  - The application payload for the Bleeper platform is as follows:  
    LoRaMac port 1:

          { 0x00/0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
           ----------  ----------  ----------  ----------  ----
               |           |           |           |        |
              LED      PRESSURE   TEMPERATURE  ALTITUDE  BATTERY
                                             (barometric)

  - The application payload for the LoRaMote platform is as follows:  
    LoRaMac port 2:

          { 0x00/0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
            ----------  ----------  ----------  ----------  ----  ----------------  ----------------  ----------
                |           |           |           |        |           |                 |              |
               LED      PRESSURE   TEMPERATURE  ALTITUDE  BATTERY    LATITUDE          LONGITUDE      ALTITUDE
                                              (barometric)                                              (gps)
- Changed applications examples according to the new MAC layer API
- MAC commands implemented
  - LinkCheckReq                 **YES**
  - LinkCheckAns                 **YES**
  - LinkADRReq                   **YES**
  - LinkADRAns                   **YES**
  - DevStatusReq                 **YES**
  - DevStatusAns                 **YES**
  - JoinReq                      **YES (Not tested)**
  - JoinAccept                   **YES (Not tested)**

### Known bugs

- Still some issues with RTC/Timer. Will be solved in a future version

## [1.1.0] - 2014-01-24

### General

- GitHub reported issues corrections

### Added

- Implemented an application LED control
  - If the server sends on port 1 an application payload of one byte with the following contents:

        0: LED off
        1: LED on
 - The node transmits periodically on port 1 the LED status on 1st byte and the message "Hello World!!!!"
   the array looks lik

        { 0, 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!' }

### Changed

- MAC commands implemented
  - LinkCheckReq                 **NO**
  - LinkCheckAns                 **NO**
  - LinkADRReq                   **YES**
  - LinkADRAns                   **YES**
  - DevStatusReq                 **YES**
  - DevStatusAns                 **YES**

### Known bugs

- There is an issue when launching an asynchronous Timer. Will be solved in a future version

## [1.1.0-RC1] - 2014-01-20

### Added

- Added Doc directory. The directory contains:
  - LoRa MAC specification
  - Bleeper board schematic

### Changed

- Changed LoRaMac according to Release1 of the specification. Main changes are:
  - MAC API changed
  - Frame format
  - ClassA first ADR implementation
  - MAC commands implemented
    - LinkCheckReq              **NO**
    - LinkCheckAns              **NO**
    - LinkADRReq                **YES**
    - LinkADRAns                **NO**
    - DevStatusReq              **NO**
    - DevStatusAns              **NO**
- Timers and RTC rewriting. Known bugs are:
  - The Radio wakeup time is taken in account for all timings
  - When opening the second reception window the microcontroller sometimes doesn't enter in low power mode

## [1.0.0] - 2013-11-28

### General

- Initial version of the LoRa MAC node firmware implementation

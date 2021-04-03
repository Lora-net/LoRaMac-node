/*
	This is a library written for the Ublox ZED-F9P and NEO-M8P-2
	SparkFun sells these at its website: www.sparkfun.com
	Do you like this library? Help support SparkFun. Buy a board!
	https://www.sparkfun.com/products/15136
	https://www.sparkfun.com/products/15005
	https://www.sparkfun.com/products/15733
	https://www.sparkfun.com/products/15193
	https://www.sparkfun.com/products/15210

	Written by Nathan Seidle @ SparkFun Electronics, September 6th, 2018

	This library handles configuring and handling the responses
	from a Ublox GPS module. Works with most modules from Ublox including
	the Zed-F9P, NEO-M8P-2, NEO-M9N, ZOE-M8Q, SAM-M8Q, and many others.

	https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library

	Development environment specifics:
	Arduino IDE 1.8.5

	SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
	The MIT License (MIT)
	Copyright (c) 2016 SparkFun Electronics
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
	associated documentation files (the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to
	do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial
	portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
	NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef SPARKFUN_UBLOX_ARDUINO_LIBRARY_H
#define SPARKFUN_UBLOX_ARDUINO_LIBRARY_H

#include <stdint.h>
#include <stdbool.h>

//#include <Wire.h>

//Platform specific configurations

//Define the size of the I2C buffer based on the platform the user has
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

//I2C_BUFFER_LENGTH is defined in Wire.H
#define I2C_BUFFER_LENGTH BUFFER_LENGTH

#elif defined(__SAMD21G18A__)

//SAMD21 uses RingBuffer.h
#define I2C_BUFFER_LENGTH SERIAL_BUFFER_SIZE

//#elif __MK20DX256__
//Teensy

#endif

#ifndef I2C_BUFFER_LENGTH

//The catch-all default is 32
#define I2C_BUFFER_LENGTH 32
//#define I2C_BUFFER_LENGTH 16 //For testing on Artemis

#endif

// Define Serial for SparkFun SAMD based boards.
// Boards like the RedBoard Turbo use SerialUSB (not Serial).
// But other boards like the SAMD51 Thing Plus use Serial (not SerialUSB).
// The next nine lines let the code compile cleanly on as many SAMD boards as possible.
#if defined(ARDUINO_ARCH_SAMD) // Is this a SAMD board?
	#if defined(USB_VID) // Is the USB Vendor ID defined?
		#if (USB_VID == 0x1B4F) // Is this a SparkFun board?
			#if !defined(ARDUINO_SAMD51_THING_PLUS) // If it is not a SAMD51 Thing Plus
				#define Serial SerialUSB // Define Serial as SerialUSB
			#endif
		#endif
	#endif
#endif
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Define a digital pin to aid checksum failure capture and analysis
//Leave set to -1 if not needed
static const int checksumFailurePin = -1;

// Global Status Returns
typedef enum
{
	SFE_UBLOX_STATUS_SUCCESS,
	SFE_UBLOX_STATUS_FAIL,
	SFE_UBLOX_STATUS_CRC_FAIL,
	SFE_UBLOX_STATUS_TIMEOUT,
	SFE_UBLOX_STATUS_COMMAND_NACK, // Indicates that the command was unrecognised, invalid or that the module is too busy to respond
	SFE_UBLOX_STATUS_OUT_OF_RANGE,
	SFE_UBLOX_STATUS_INVALID_ARG,
	SFE_UBLOX_STATUS_INVALID_OPERATION,
	SFE_UBLOX_STATUS_MEM_ERR,
	SFE_UBLOX_STATUS_HW_ERR,
	SFE_UBLOX_STATUS_DATA_SENT,		// This indicates that a 'set' was successful
	SFE_UBLOX_STATUS_DATA_RECEIVED, // This indicates that a 'get' (poll) was successful
	SFE_UBLOX_STATUS_I2C_COMM_FAILURE,
	SFE_UBLOX_STATUS_DATA_OVERWRITTEN // This is an error - the data was valid but has been or _is being_ overwritten by another packet
} sfe_ublox_status_e;

// ubxPacket validity
typedef enum
{
	SFE_UBLOX_PACKET_VALIDITY_NOT_VALID,
	SFE_UBLOX_PACKET_VALIDITY_VALID,
	SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED,
	SFE_UBLOX_PACKET_NOTACKNOWLEDGED // This indicates that we received a NACK
} sfe_ublox_packet_validity_e;

// Identify which packet buffer is in use:
// packetCfg (or a custom packet), packetAck or packetBuf
typedef enum
{
	SFE_UBLOX_PACKET_PACKETCFG,
	SFE_UBLOX_PACKET_PACKETACK,
	SFE_UBLOX_PACKET_PACKETBUF
} sfe_ublox_packet_buffer_e;

//Registers
static const uint8_t UBX_SYNCH_1 = 0xB5;
static const uint8_t UBX_SYNCH_2 = 0x62;

//The following are UBX Class IDs. Descriptions taken from ZED-F9P Interface Description Document page 32, NEO-M8P Interface Description page 145
static const uint8_t UBX_CLASS_NAV = 0x01;	 //Navigation Results Messages: Position, Speed, Time, Acceleration, Heading, DOP, SVs used
static const uint8_t UBX_CLASS_RXM = 0x02;	 //Receiver Manager Messages: Satellite Status, RTC Status
static const uint8_t UBX_CLASS_INF = 0x04;	 //Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice
static const uint8_t UBX_CLASS_ACK = 0x05;	 //Ack/Nak Messages: Acknowledge or Reject messages to UBX-CFG input messages
static const uint8_t UBX_CLASS_CFG = 0x06;	 //Configuration Input Messages: Configure the receiver.
static const uint8_t UBX_CLASS_UPD = 0x09;	 //Firmware Update Messages: Memory/Flash erase/write, Reboot, Flash identification, etc.
static const uint8_t UBX_CLASS_MON = 0x0A;	 //Monitoring Messages: Communication Status, CPU Load, Stack Usage, Task Status
static const uint8_t UBX_CLASS_AID = 0x0B;	 //(NEO-M8P ONLY!!!) AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input
static const uint8_t UBX_CLASS_TIM = 0x0D;	 //Timing Messages: Time Pulse Output, Time Mark Results
static const uint8_t UBX_CLASS_ESF = 0x10;	 //(NEO-M8P ONLY!!!) External Sensor Fusion Messages: External Sensor Measurements and Status Information
static const uint8_t UBX_CLASS_MGA = 0x13;	 //Multiple GNSS Assistance Messages: Assistance data for various GNSS
static const uint8_t UBX_CLASS_LOG = 0x21;	 //Logging Messages: Log creation, deletion, info and retrieval
static const uint8_t UBX_CLASS_SEC = 0x27;	 //Security Feature Messages
static const uint8_t UBX_CLASS_HNR = 0x28;	 //(NEO-M8P ONLY!!!) High Rate Navigation Results Messages: High rate time, position speed, heading
static const uint8_t UBX_CLASS_NMEA = 0xF0; //NMEA Strings: standard NMEA strings

//The following are used for configuration. Descriptions are from the ZED-F9P Interface Description pg 33-34 and NEO-M9N Interface Description pg 47-48
static const uint8_t UBX_CFG_ANT = 0x13;		//Antenna Control Settings. Used to configure the antenna control settings
static const uint8_t UBX_CFG_BATCH = 0x93;		//Get/set data batching configuration.
static const uint8_t UBX_CFG_CFG = 0x09;		//Clear, Save, and Load Configurations. Used to save current configuration
static const uint8_t UBX_CFG_DAT = 0x06;		//Set User-defined Datum or The currently defined Datum
static const uint8_t UBX_CFG_DGNSS = 0x70;		//DGNSS configuration
static const uint8_t UBX_CFG_GEOFENCE = 0x69;	//Geofencing configuration. Used to configure a geofence
static const uint8_t UBX_CFG_GNSS = 0x3E;		//GNSS system configuration
static const uint8_t UBX_CFG_INF = 0x02;		//Depending on packet length, either: poll configuration for one protocol, or information message configuration
static const uint8_t UBX_CFG_ITFM = 0x39;		//Jamming/Interference Monitor configuration
static const uint8_t UBX_CFG_LOGFILTER = 0x47; //Data Logger Configuration
static const uint8_t UBX_CFG_MSG = 0x01;		//Poll a message configuration, or Set Message Rate(s), or Set Message Rate
static const uint8_t UBX_CFG_NAV5 = 0x24;		//Navigation Engine Settings. Used to configure the navigation engine including the dynamic model.
static const uint8_t UBX_CFG_NAVX5 = 0x23;		//Navigation Engine Expert Settings
static const uint8_t UBX_CFG_NMEA = 0x17;		//Extended NMEA protocol configuration V1
static const uint8_t UBX_CFG_ODO = 0x1E;		//Odometer, Low-speed COG Engine Settings
static const uint8_t UBX_CFG_PM2 = 0x3B;		//Extended power management configuration
static const uint8_t UBX_CFG_PMS = 0x86;		//Power mode setup
static const uint8_t UBX_CFG_PRT = 0x00;		//Used to configure port specifics. Polls the configuration for one I/O Port, or Port configuration for UART ports, or Port configuration for USB port, or Port configuration for SPI port, or Port configuration for DDC port
static const uint8_t UBX_CFG_PWR = 0x57;		//Put receiver in a defined power state
static const uint8_t UBX_CFG_RATE = 0x08;		//Navigation/Measurement Rate Settings. Used to set port baud rates.
static const uint8_t UBX_CFG_RINV = 0x34;		//Contents of Remote Inventory
static const uint8_t UBX_CFG_RST = 0x04;		//Reset Receiver / Clear Backup Data Structures. Used to reset device.
static const uint8_t UBX_CFG_RXM = 0x11;		//RXM configuration
static const uint8_t UBX_CFG_SBAS = 0x16;		//SBAS configuration
static const uint8_t UBX_CFG_TMODE3 = 0x71;	//Time Mode Settings 3. Used to enable Survey In Mode
static const uint8_t UBX_CFG_TP5 = 0x31;		//Time Pulse Parameters
static const uint8_t UBX_CFG_USB = 0x1B;		//USB Configuration
static const uint8_t UBX_CFG_VALDEL = 0x8C;	//Used for config of higher version Ublox modules (ie protocol v27 and above). Deletes values corresponding to provided keys/ provided keys with a transaction
static const uint8_t UBX_CFG_VALGET = 0x8B;	//Used for config of higher version Ublox modules (ie protocol v27 and above). Configuration Items
static const uint8_t UBX_CFG_VALSET = 0x8A;	//Used for config of higher version Ublox modules (ie protocol v27 and above). Sets values corresponding to provided key-value pairs/ provided key-value pairs within a transaction.

//The following are used to enable NMEA messages. Descriptions come from the NMEA messages overview in the ZED-F9P Interface Description
static const uint8_t UBX_NMEA_MSB = 0xF0;	//All NMEA enable commands have 0xF0 as MSB
static const uint8_t UBX_NMEA_DTM = 0x0A;	//GxDTM (datum reference)
static const uint8_t UBX_NMEA_GAQ = 0x45;	//GxGAQ (poll a standard message (if the current talker ID is GA))
static const uint8_t UBX_NMEA_GBQ = 0x44;	//GxGBQ (poll a standard message (if the current Talker ID is GB))
static const uint8_t UBX_NMEA_GBS = 0x09;	//GxGBS (GNSS satellite fault detection)
static const uint8_t UBX_NMEA_GGA = 0x00;	//GxGGA (Global positioning system fix data)
static const uint8_t UBX_NMEA_GLL = 0x01;	//GxGLL (latitude and long, whith time of position fix and status)
static const uint8_t UBX_NMEA_GLQ = 0x43;	//GxGLQ (poll a standard message (if the current Talker ID is GL))
static const uint8_t UBX_NMEA_GNQ = 0x42;	//GxGNQ (poll a standard message (if the current Talker ID is GN))
static const uint8_t UBX_NMEA_GNS = 0x0D;	//GxGNS (GNSS fix data)
static const uint8_t UBX_NMEA_GPQ = 0x040; //GxGPQ (poll a standard message (if the current Talker ID is GP))
static const uint8_t UBX_NMEA_GRS = 0x06;	//GxGRS (GNSS range residuals)
static const uint8_t UBX_NMEA_GSA = 0x02;	//GxGSA (GNSS DOP and Active satellites)
static const uint8_t UBX_NMEA_GST = 0x07;	//GxGST (GNSS Pseudo Range Error Statistics)
static const uint8_t UBX_NMEA_GSV = 0x03;	//GxGSV (GNSS satellites in view)
static const uint8_t UBX_NMEA_RMC = 0x04;	//GxRMC (Recommended minimum data)
static const uint8_t UBX_NMEA_TXT = 0x41;	//GxTXT (text transmission)
static const uint8_t UBX_NMEA_VLW = 0x0F;	//GxVLW (dual ground/water distance)
static const uint8_t UBX_NMEA_VTG = 0x05;	//GxVTG (course over ground and Ground speed)
static const uint8_t UBX_NMEA_ZDA = 0x08;	//GxZDA (Time and Date)

//The following are used to configure the NMEA protocol main talker ID and GSV talker ID
static const uint8_t UBX_NMEA_MAINTALKERID_NOTOVERRIDDEN = 0x00; //main talker ID is system dependent
static const uint8_t UBX_NMEA_MAINTALKERID_GP = 0x01;			  //main talker ID is GPS
static const uint8_t UBX_NMEA_MAINTALKERID_GL = 0x02;			  //main talker ID is GLONASS
static const uint8_t UBX_NMEA_MAINTALKERID_GN = 0x03;			  //main talker ID is combined receiver
static const uint8_t UBX_NMEA_MAINTALKERID_GA = 0x04;			  //main talker ID is Galileo
static const uint8_t UBX_NMEA_MAINTALKERID_GB = 0x05;			  //main talker ID is BeiDou
static const uint8_t UBX_NMEA_GSVTALKERID_GNSS = 0x00;			  //GNSS specific Talker ID (as defined by NMEA)
static const uint8_t UBX_NMEA_GSVTALKERID_MAIN = 0x01;			  //use the main Talker ID

//The following are used to configure INF UBX messages (information messages).  Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 34)
static const uint8_t UBX_INF_CLASS = 0x04;	  //All INF messages have 0x04 as the class
static const uint8_t UBX_INF_DEBUG = 0x04;	  //ASCII output with debug contents
static const uint8_t UBX_INF_ERROR = 0x00;	  //ASCII output with error contents
static const uint8_t UBX_INF_NOTICE = 0x02;  //ASCII output with informational contents
static const uint8_t UBX_INF_TEST = 0x03;	  //ASCII output with test contents
static const uint8_t UBX_INF_WARNING = 0x01; //ASCII output with warning contents

//The following are used to configure LOG UBX messages (loggings messages).  Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 34)
static const uint8_t UBX_LOG_CREATE = 0x07;		   //Create Log File
static const uint8_t UBX_LOG_ERASE = 0x03;			   //Erase Logged Data
static const uint8_t UBX_LOG_FINDTIME = 0x0E;		   //Find index of a log entry based on a given time, or response to FINDTIME requested
static const uint8_t UBX_LOG_INFO = 0x08;			   //Poll for log information, or Log information
static const uint8_t UBX_LOG_RETRIEVEPOSEXTRA = 0x0F; //Odometer log entry
static const uint8_t UBX_LOG_RETRIEVEPOS = 0x0B;	   //Position fix log entry
static const uint8_t UBX_LOG_RETRIEVESTRING = 0x0D;   //Byte string log entry
static const uint8_t UBX_LOG_RETRIEVE = 0x09;		   //Request log data
static const uint8_t UBX_LOG_STRING = 0x04;		   //Store arbitrary string on on-board flash

//The following are used to configure MGA UBX messages (Multiple GNSS Assistance Messages).  Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 34)
static const uint8_t UBX_MGA_ACK_DATA0 = 0x60;		 //Multiple GNSS Acknowledge message
static const uint8_t UBX_MGA_BDS_EPH = 0x03;		 //BDS Ephemeris Assistance
static const uint8_t UBX_MGA_BDS_ALM = 0x03;		 //BDS Almanac Assistance
static const uint8_t UBX_MGA_BDS_HEALTH = 0x03;	 //BDS Health Assistance
static const uint8_t UBX_MGA_BDS_UTC = 0x03;		 //BDS UTC Assistance
static const uint8_t UBX_MGA_BDS_IONO = 0x03;		 //BDS Ionospheric Assistance
static const uint8_t UBX_MGA_DBD = 0x80;			 //Either: Poll the Navigation Database, or Navigation Database Dump Entry
static const uint8_t UBX_MGA_GAL_EPH = 0x02;		 //Galileo Ephemeris Assistance
static const uint8_t UBX_MGA_GAL_ALM = 0x02;		 //Galileo Almanac Assitance
static const uint8_t UBX_MGA_GAL_TIMOFFSET = 0x02;	 //Galileo GPS time offset assistance
static const uint8_t UBX_MGA_GAL_UTC = 0x02;		 //Galileo UTC Assistance
static const uint8_t UBX_MGA_GLO_EPH = 0x06;		 //GLONASS Ephemeris Assistance
static const uint8_t UBX_MGA_GLO_ALM = 0x06;		 //GLONASS Almanac Assistance
static const uint8_t UBX_MGA_GLO_TIMEOFFSET = 0x06; //GLONASS Auxiliary Time Offset Assistance
static const uint8_t UBX_MGA_GPS_EPH = 0x00;		 //GPS Ephemeris Assistance
static const uint8_t UBX_MGA_GPS_ALM = 0x00;		 //GPS Almanac Assistance
static const uint8_t UBX_MGA_GPS_HEALTH = 0x00;	 //GPS Health Assistance
static const uint8_t UBX_MGA_GPS_UTC = 0x00;		 //GPS UTC Assistance
static const uint8_t UBX_MGA_GPS_IONO = 0x00;		 //GPS Ionosphere Assistance
static const uint8_t UBX_MGA_INI_POS_XYZ = 0x40;	 //Initial Position Assistance
static const uint8_t UBX_MGA_INI_POS_LLH = 0x40;	 //Initial Position Assitance
static const uint8_t UBX_MGA_INI_TIME_UTC = 0x40;	 //Initial Time Assistance
static const uint8_t UBX_MGA_INI_TIME_GNSS = 0x40;	 //Initial Time Assistance
static const uint8_t UBX_MGA_INI_CLKD = 0x40;		 //Initial Clock Drift Assitance
static const uint8_t UBX_MGA_INI_FREQ = 0x40;		 //Initial Frequency Assistance
static const uint8_t UBX_MGA_INI_EOP = 0x40;		 //Earth Orientation Parameters Assistance
static const uint8_t UBX_MGA_QZSS_EPH = 0x05;		 //QZSS Ephemeris Assistance
static const uint8_t UBX_MGA_QZSS_ALM = 0x05;		 //QZSS Almanac Assistance
static const uint8_t UBX_MGA_QZAA_HEALTH = 0x05;	 //QZSS Health Assistance

//The following are used to configure the MON UBX messages (monitoring messages). Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 35)
static const uint8_t UBX_MON_COMMS = 0x36; //Comm port information
static const uint8_t UBX_MON_GNSS = 0x28;	//Information message major GNSS selection
static const uint8_t UBX_MON_HW2 = 0x0B;	//Extended Hardware Status
static const uint8_t UBX_MON_HW3 = 0x37;	//HW I/O pin information
static const uint8_t UBX_MON_HW = 0x09;	//Hardware Status
static const uint8_t UBX_MON_IO = 0x02;	//I/O Subsystem Status
static const uint8_t UBX_MON_MSGPP = 0x06; //Message Parse and Process Status
static const uint8_t UBX_MON_PATCH = 0x27; //Output information about installed patches
static const uint8_t UBX_MON_RF = 0x38;	//RF information
static const uint8_t UBX_MON_RXBUF = 0x07; //Receiver Buffer Status
static const uint8_t UBX_MON_RXR = 0x21;	//Receiver Status Information
static const uint8_t UBX_MON_TXBUF = 0x08; //Transmitter Buffer Status. Used for query tx buffer size/state.
static const uint8_t UBX_MON_VER = 0x04;	//Receiver/Software Version. Used for obtaining Protocol Version.

//The following are used to configure the NAV UBX messages (navigation results messages). Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 35-36)
static const uint8_t UBX_NAV_ATT = 0x05;		//Vehicle "Attitude" Solution
static const uint8_t UBX_NAV_CLOCK = 0x22;		//Clock Solution
static const uint8_t UBX_NAV_DOP = 0x04;		//Dilution of precision
static const uint8_t UBX_NAV_EOE = 0x61;		//End of Epoch
static const uint8_t UBX_NAV_GEOFENCE = 0x39;	//Geofencing status. Used to poll the geofence status
static const uint8_t UBX_NAV_HPPOSECEF = 0x13; //High Precision Position Solution in ECEF. Used to find our positional accuracy (high precision).
static const uint8_t UBX_NAV_HPPOSLLH = 0x14;	//High Precision Geodetic Position Solution. Used for obtaining lat/long/alt in high precision
static const uint8_t UBX_NAV_ODO = 0x09;		//Odometer Solution
static const uint8_t UBX_NAV_ORB = 0x34;		//GNSS Orbit Database Info
static const uint8_t UBX_NAV_POSECEF = 0x01;	//Position Solution in ECEF
static const uint8_t UBX_NAV_POSLLH = 0x02;	//Geodetic Position Solution
static const uint8_t UBX_NAV_PVT = 0x07;		//All the things! Position, velocity, time, PDOP, height, h/v accuracies, number of satellites. Navigation Position Velocity Time Solution.
static const uint8_t UBX_NAV_RELPOSNED = 0x3C; //Relative Positioning Information in NED frame
static const uint8_t UBX_NAV_RESETODO = 0x10;	//Reset odometer
static const uint8_t UBX_NAV_SAT = 0x35;		//Satellite Information
static const uint8_t UBX_NAV_SIG = 0x43;		//Signal Information
static const uint8_t UBX_NAV_STATUS = 0x03;	//Receiver Navigation Status
static const uint8_t UBX_NAV_SVIN = 0x3B;		//Survey-in data. Used for checking Survey In status
static const uint8_t UBX_NAV_TIMEBDS = 0x24;	//BDS Time Solution
static const uint8_t UBX_NAV_TIMEGAL = 0x25;	//Galileo Time Solution
static const uint8_t UBX_NAV_TIMEGLO = 0x23;	//GLO Time Solution
static const uint8_t UBX_NAV_TIMEGPS = 0x20;	//GPS Time Solution
static const uint8_t UBX_NAV_TIMELS = 0x26;	//Leap second event information
static const uint8_t UBX_NAV_TIMEUTC = 0x21;	//UTC Time Solution
static const uint8_t UBX_NAV_VELECEF = 0x11;	//Velocity Solution in ECEF
static const uint8_t UBX_NAV_VELNED = 0x12;	//Velocity Solution in NED

//The following are used to configure the RXM UBX messages (receiver manager messages). Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 36)
static const uint8_t UBX_RXM_MEASX = 0x14; //Satellite Measurements for RRLP
static const uint8_t UBX_RXM_PMREQ = 0x41; //Requests a Power Management task (two differenent packet sizes)
static const uint8_t UBX_RXM_RAWX = 0x15;	//Multi-GNSS Raw Measurement Data
static const uint8_t UBX_RXM_RLM = 0x59;	//Galileo SAR Short-RLM report (two different packet sizes)
static const uint8_t UBX_RXM_RTCM = 0x32;	//RTCM input status
static const uint8_t UBX_RXM_SFRBX = 0x13; //Boradcast Navigation Data Subframe

//The following are used to configure the SEC UBX messages (security feature messages). Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 36)
static const uint8_t UBX_SEC_UNIQID = 0x03; //Unique chip ID

//The following are used to configure the TIM UBX messages (timing messages). Descriptions from UBX messages overview (ZED_F9P Interface Description Document page 36)
static const uint8_t UBX_TIM_TM2 = 0x03;  //Time mark data
static const uint8_t UBX_TIM_TP = 0x01;   //Time Pulse Timedata
static const uint8_t UBX_TIM_VRFY = 0x06; //Sourced Time Verification

//The following are used to configure the UPD UBX messages (firmware update messages). Descriptions from UBX messages overview (ZED-F9P Interface Description Document page 36)
static const uint8_t UBX_UPD_SOS = 0x14; //Poll Backup Fil Restore Status, Create Backup File in Flash, Clear Backup File in Flash, Backup File Creation Acknowledge, System Restored from Backup

//The following are used to enable RTCM messages
static const uint8_t UBX_RTCM_MSB = 0xF5;	  //All RTCM enable commands have 0xF5 as MSB
static const uint8_t UBX_RTCM_1005 = 0x05;	  //Stationary RTK reference ARP
static const uint8_t UBX_RTCM_1074 = 0x4A;	  //GPS MSM4
static const uint8_t UBX_RTCM_1077 = 0x4D;	  //GPS MSM7
static const uint8_t UBX_RTCM_1084 = 0x54;	  //GLONASS MSM4
static const uint8_t UBX_RTCM_1087 = 0x57;	  //GLONASS MSM7
static const uint8_t UBX_RTCM_1094 = 0x5E;	  //Galileo MSM4
static const uint8_t UBX_RTCM_1097 = 0x61;	  //Galileo MSM7
static const uint8_t UBX_RTCM_1124 = 0x7C;	  //BeiDou MSM4
static const uint8_t UBX_RTCM_1127 = 0x7F;	  //BeiDou MSM7
static const uint8_t UBX_RTCM_1230 = 0xE6;	  //GLONASS code-phase biases, set to once every 10 seconds
static const uint8_t UBX_RTCM_4072_0 = 0xFE; //Reference station PVT (ublox proprietary RTCM message)
static const uint8_t UBX_RTCM_4072_1 = 0xFD; //Additional reference station information (ublox proprietary RTCM message)

static const uint8_t UBX_ACK_NACK = 0x00;
static const uint8_t UBX_ACK_ACK = 0x01;
static const uint8_t UBX_ACK_NONE = 0x02; //Not a real value

// The following constants are used to get External Sensor Measurements and Status
// Information.
static const uint8_t UBX_ESF_MEAS = 0x02;
static const uint8_t UBX_ESF_RAW = 0x03;
static const uint8_t UBX_ESF_STATUS = 0x10;
static const uint8_t UBX_ESF_INS = 0x15; //36 bytes

static const uint8_t SVIN_MODE_DISABLE = 0x00;
static const uint8_t SVIN_MODE_ENABLE = 0x01;

//The following consts are used to configure the various ports and streams for those ports. See -CFG-PRT.
static const uint8_t COM_PORT_I2C = 0;
static const uint8_t COM_PORT_UART1 = 1;
static const uint8_t COM_PORT_UART2 = 2;
static const uint8_t COM_PORT_USB = 3;
static const uint8_t COM_PORT_SPI = 4;

static const uint8_t COM_TYPE_UBX = (1 << 0);
static const uint8_t COM_TYPE_NMEA = (1 << 1);
static const uint8_t COM_TYPE_RTCM3 = (1 << 5);

//The following consts are used to generate KEY values for the advanced protocol functions of VELGET/SET/DEL
static const uint8_t VAL_SIZE_1 = 0x01;  //One bit
static const uint8_t VAL_SIZE_8 = 0x02;  //One byte
static const uint8_t VAL_SIZE_16 = 0x03; //Two bytes
static const uint8_t VAL_SIZE_32 = 0x04; //Four bytes
static const uint8_t VAL_SIZE_64 = 0x05; //Eight bytes

//These are the Bitfield layers definitions for the UBX-CFG-VALSET message (not to be confused with Bitfield deviceMask in UBX-CFG-CFG)
static const uint8_t VAL_LAYER_RAM = (1 << 0);
static const uint8_t VAL_LAYER_BBR = (1 << 1);
static const uint8_t VAL_LAYER_FLASH = (1 << 2);

//Below are various Groups, IDs, and sizes for various settings
//These can be used to call getVal/setVal/delVal
static const uint8_t VAL_GROUP_I2COUTPROT = 0x72;
static const uint8_t VAL_GROUP_I2COUTPROT_SIZE = VAL_SIZE_1; //All fields in I2C group are currently 1 bit

static const uint8_t VAL_ID_I2COUTPROT_UBX = 0x01;
static const uint8_t VAL_ID_I2COUTPROT_NMEA = 0x02;
static const uint8_t VAL_ID_I2COUTPROT_RTCM3 = 0x03;

static const uint8_t VAL_GROUP_I2C = 0x51;
static const uint8_t VAL_GROUP_I2C_SIZE = VAL_SIZE_8; //All fields in I2C group are currently 1 byte

static const uint8_t VAL_ID_I2C_ADDRESS = 0x01;

// Configuration Sub-Section mask definitions for saveConfigSelective (UBX-CFG-CFG)
static const uint32_t VAL_CFG_SUBSEC_IOPORT = 0x00000001;	 // ioPort - communications port settings (causes IO system reset!)
static const uint32_t VAL_CFG_SUBSEC_MSGCONF = 0x00000002;	 // msgConf - message configuration
static const uint32_t VAL_CFG_SUBSEC_INFMSG = 0x00000004;	 // infMsg - INF message configuration
static const uint32_t VAL_CFG_SUBSEC_NAVCONF = 0x00000008;	 // navConf - navigation configuration
static const uint32_t VAL_CFG_SUBSEC_RXMCONF = 0x00000010;	 // rxmConf - receiver manager configuration
static const uint32_t VAL_CFG_SUBSEC_SENCONF = 0x00000100;	 // senConf - sensor interface configuration (requires protocol 19+)
static const uint32_t VAL_CFG_SUBSEC_RINVCONF = 0x00000200; // rinvConf - remove inventory configuration
static const uint32_t VAL_CFG_SUBSEC_ANTCONF = 0x00000400;	 // antConf - antenna configuration
static const uint32_t VAL_CFG_SUBSEC_LOGCONF = 0x00000800;	 // logConf - logging configuration
static const uint32_t VAL_CFG_SUBSEC_FTSCONF = 0x00001000;	 // ftsConf - FTS configuration (FTS products only)


// Possible values for the dynamic platform model, which provide more accuract position output for the situation. Description extracted from ZED-F9P Integration Manual;

typedef enum 
{
	DYN_MODEL_PORTABLE = 0, //Applications with low acceleration, e.g. portable devices. Suitable for most situations.
	// 1 is not defined
	DYN_MODEL_STATIONARY = 2, //Used in timing applications (antenna must be stationary) or other stationary applications. Velocity restricted to 0 m/s. Zero dynamics assumed.
	DYN_MODEL_PEDESTRIAN,	  //Applications with low acceleration and speed, e.g. how a pedestrian would move. Low acceleration assumed.
	DYN_MODEL_AUTOMOTIVE,	  //Used for applications with equivalent dynamics to those of a passenger car. Low vertical acceleration assumed
	DYN_MODEL_SEA,			  //Recommended for applications at sea, with zero vertical velocity. Zero vertical velocity assumed. Sea level assumed.
	DYN_MODEL_AIRBORNE1g,	  //Airborne <1g acceleration. Used for applications with a higher dynamic range and greater vertical acceleration than a passenger car. No 2D position fixes supported.
	DYN_MODEL_AIRBORNE2g,	  //Airborne <2g acceleration. Recommended for typical airborne environments. No 2D position fixes supported.
	DYN_MODEL_AIRBORNE4g,	  //Airborne <4g acceleration. Only recommended for extremely dynamic environments. No 2D position fixes supported.
	DYN_MODEL_WRIST,		  // Not supported in protocol versions less than 18. Only recommended for wrist worn applications. Receiver will filter out arm motion.
	DYN_MODEL_BIKE,			  // Supported in protocol versions 19.2
} dynModel; 
#ifndef MAX_PAYLOAD_SIZE

//#define MAX_PAYLOAD_SIZE 256 //We need ~220 bytes for getProtocolVersion on most ublox modules
#define MAX_PAYLOAD_SIZE 768 //Worst case: UBX_CFG_VALSET packet with 64 keyIDs each with 64 bit values

#endif

//-=-=-=-=- UBX binary specific variables
typedef struct
{
	uint8_t cls;
	uint8_t id;
	uint16_t len;		   //Length of the payload. Does not include cls, id, or checksum bytes
	uint16_t counter;	   //Keeps track of number of overall bytes received. Some responses are larger than 255 bytes.
	uint16_t startingSpot; //The counter value needed to go past before we begin recording into payload array
	uint8_t *payload;
	uint8_t checksumA; //Given to us from module. Checked against the rolling calculated A/B checksums.
	uint8_t checksumB;
	sfe_ublox_packet_validity_e valid;			 //Goes from NOT_DEFINED to VALID or NOT_VALID when checksum is checked
	sfe_ublox_packet_validity_e classAndIDmatch; // Goes from NOT_DEFINED to VALID or NOT_VALID when the Class and ID match the requestedClass and requestedID
} ubxPacket;

// Struct to hold the results returned by getGeofenceState (returned by UBX-NAV-GEOFENCE)
typedef struct
{
	uint8_t status;	   // Geofencing status: 0 - Geofencing not available or not reliable; 1 - Geofencing active
	uint8_t numFences; // Number of geofences
	uint8_t combState; // Combined (logical OR) state of all geofences: 0 - Unknown; 1 - Inside; 2 - Outside
	uint8_t states[4]; // Geofence states: 0 - Unknown; 1 - Inside; 2 - Outside
} geofenceState;

// Struct to hold the current geofence parameters
typedef struct
{
	uint8_t numFences; // Number of active geofences
	int32_t lats[4];   // Latitudes of geofences (in degrees * 10^-7)
	int32_t longs[4];  // Longitudes of geofences (in degrees * 10^-7)
	uint32_t rads[4];  // Radii of geofences (in m * 10^-2)
} geofenceParams;




// A default of 250ms for maxWait seems fine for I2C but is not enough for SerialUSB.
// If you know you are only going to be using I2C / Qwiic communication, you can
// safely reduce defaultMaxWait to 250.
#ifndef defaultMaxWait // Let's allow the user to define their own value if they want to
#define defaultMaxWait 1000
#endif

	//By default use the default I2C address, and use Wire port
	//bool begin(TwoWire &wirePort = Wire, uint8_t deviceAddress = 0x42); //Returns true if module is detected
	//serialPort needs to be perviously initialized to correct baud rate

	//Returns true if device answers on _gpsI2Caddress address or via Serial
	//maxWait is only used for Serial
	bool isConnected(uint16_t maxWait);

	//Changed in V1.8.1: provides backward compatibility for the examples that call checkUblox directly
	//Will default to using packetCfg to look for explicit autoPVT packets so they get processed correctly by processUBX
	bool checkUblox(uint8_t requestedClass, uint8_t requestedID); //Checks module with user selected commType

	bool checkUbloxI2C(ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID);	   //Method for I2C polling of data, passing any new bytes to process()
	bool checkUbloxSerial(ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID); //Method for serial polling of data, passing any new bytes to process()

	void process(uint8_t incoming, ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID);	//Processes NMEA and UBX binary sentences one byte at a time
	void processUBX(uint8_t incoming, ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID); //Given a character, file it away into the uxb packet structure
	void processRTCMframe(uint8_t incoming);																//Monitor the incoming bytes for start and length bytes
	void processRTCM(uint8_t incoming) __attribute__((weak));												//Given rtcm byte, do something with it. User can overwrite if desired to pipe bytes to radio, internet, etc.

	void processUBXpacket(ubxPacket *msg);				   //Once a packet has been received and validated, identify this packet's class/id and update internal flags
	void processNMEA(char incoming) __attribute__((weak)); //Given a NMEA character, do something with it. User can overwrite if desired to use something like tinyGPS or MicroNMEA libraries

	void calcChecksum(ubxPacket *msg);														   //Sets the checksumA and checksumB of a given messages
	sfe_ublox_status_e sendCommand(ubxPacket *outgoingUBX, uint16_t maxWait ); //Given a packet and payload, send everything including CRC bytes, return true if we got a response
	sfe_ublox_status_e sendI2cCommand(ubxPacket *outgoingUBX, uint16_t maxWait );
	void sendSerialCommand(ubxPacket *outgoingUBX);

	void printPacket(ubxPacket *packet); //Useful for debugging

	void factoryReset(void); //Send factory reset sequence (i.e. load "default" configuration and perform hardReset)
	void hardReset(void);	 //Perform a reset leading to a cold start (zero info start-up)
	void ihardReset(void);	 //Perform a reset leading to a cold start (zero info start-up) imperial

	bool setI2CAddress(uint8_t deviceAddress, uint16_t maxTime);										 //Changes the I2C address of the Ublox module
	void setSerialRate(uint32_t baudrate, uint8_t uartPort, uint16_t maxTime ); //Changes the serial baud rate of the Ublox module, uartPort should be COM_PORT_UART1/2

	bool setNavigationFrequency(uint8_t navFreq, uint16_t maxWait);	 //Set the number of nav solutions sent per second
	uint8_t getNavigationFrequency(uint16_t maxWait );					 //Get the number of nav solutions sent per second currently being output by module
	bool saveConfiguration(uint16_t maxWait);						 //Save current configuration to flash and BBR (battery backed RAM)
	bool factoryDefault(uint16_t maxWait);							 //Reset module to factory defaults
	bool saveConfigSelective(uint32_t configMask, uint16_t maxWait); //Save the selected configuration sub-sections to flash and BBR (battery backed RAM)

	sfe_ublox_status_e waitForACKResponse(ubxPacket *outgoingUBX, uint8_t requestedClass, uint8_t requestedID, uint16_t maxTime );	 //Poll the module until a config packet and an ACK is received
	sfe_ublox_status_e waitForNoACKResponse(ubxPacket *outgoingUBX, uint8_t requestedClass, uint8_t requestedID, uint16_t maxTime ); //Poll the module until a config packet is received

// getPVT will only return data once in each navigation cycle. By default, that is once per second.
// Therefore we should set getPVTmaxWait to slightly longer than that.
// If you change the navigation frequency to (e.g.) 4Hz using setNavigationFrequency(4)
// then you should use a shorter maxWait for getPVT. 300msec would be about right: getPVT(300)
// The same is true for getHPPOSLLH.
#define getPVTmaxWait 1100		// Default maxWait for getPVT and all functions which call it
#define getHPPOSLLHmaxWait 1100 // Default maxWait for getHPPOSLLH and all functions which call it

	bool assumeAutoPVT(bool enabled, bool implicitUpdate);							//In case no config access to the GPS is possible and PVT is send cyclically already
	bool setAutoPVT(bool enabled, uint16_t maxWait );							//Enable/disable automatic PVT reports at the navigation frequency
	bool getPVT(uint16_t maxWait );												//Query module for latest group of datums and load global vars: lat, long, alt, speed, SIV, accuracies, etc. If autoPVT is disabled, performs an explicit poll and waits, if enabled does not block. Retruns true if new PVT is available.
	bool getHPPOSLLH(uint16_t maxWait );										//Query module for latest group of datums and load global vars: lat, long, alt, speed, SIV, accuracies, etc. If autoPVT is disabled, performs an explicit poll and waits, if enabled does not block. Retruns true if new PVT is available.
	void flushPVT(void);																				//Mark all the PVT data as read/stale. This is handy to get data alignment after CRC failure

	int32_t getLatitude(uint16_t maxWait );			  //Returns the current latitude in degrees * 10^-7. Auto selects between HighPrecision and Regular depending on ability of module.
	int32_t getLongitude(uint16_t maxWait );			  //Returns the current longitude in degrees * 10-7. Auto selects between HighPrecision and Regular depending on ability of module.
	int32_t getAltitude(uint16_t maxWait );			  //Returns the current altitude in mm above ellipsoid
	int32_t getAltitudeMSL(uint16_t maxWait );		  //Returns the current altitude in mm above mean sea level
	uint8_t getSIV(uint16_t maxWait );				  //Returns number of sats used in fix
	uint8_t getFixType(uint16_t maxWait );			  //Returns the type of fix: 0=no, 3=3D, 4=GNSS+Deadreckoning
	uint8_t getgnssFixOK(uint16_t maxWait );			  //Returns the fix validity: 0=not valid, 1 = fix valid
	uint8_t getCarrierSolutionType(uint16_t maxWait ); //Returns RTK solution: 0=no, 1=float solution, 2=fixed solution
	int32_t getGroundSpeed(uint16_t maxWait );		  //Returns speed in mm/s
	int32_t getHeading(uint16_t maxWait );			  //Returns heading in degrees * 10^-7
	uint16_t getPDOP(uint16_t maxWait );				  //Returns positional dillution of precision * 10^-2
	uint16_t getYear(uint16_t maxWait );
	uint8_t getMonth(uint16_t maxWait );
	uint8_t getDay(uint16_t maxWait );
	uint8_t getHour(uint16_t maxWait );
	uint8_t getMinute(uint16_t maxWait );
	uint8_t getSecond(uint16_t maxWait );
	uint16_t getMillisecond(uint16_t maxWait );
	int32_t getNanosecond(uint16_t maxWait );
	uint32_t getTimeOfWeek(uint16_t maxWait );
	bool getDateValid(uint16_t maxWait);
	bool getTimeValid(uint16_t maxWait);

	int32_t getHighResLatitude(uint16_t maxWait );
	int8_t getHighResLatitudeHp(uint16_t maxWait );
	int32_t getHighResLongitude(uint16_t maxWait );
	int8_t getHighResLongitudeHp(uint16_t maxWait );
	int32_t getElipsoid(uint16_t maxWait );
	int8_t getElipsoidHp(uint16_t maxWait );
	int32_t getMeanSeaLevel(uint16_t maxWait );
	int8_t getMeanSeaLevelHp(uint16_t maxWait );
	int32_t getGeoidSeparation(uint16_t maxWait );
	uint32_t getHorizontalAccuracy(uint16_t maxWait );
	uint32_t getVerticalAccuracy(uint16_t maxWait );

	//Port configurations
	bool setPortOutput(uint8_t portID, uint8_t comSettings, uint16_t maxWait ); //Configure a given port to output UBX, NMEA, RTCM3 or a combination thereof
	bool setPortInput(uint8_t portID, uint8_t comSettings, uint16_t maxWait );  //Configure a given port to input UBX, NMEA, RTCM3 or a combination thereof
	bool getPortSettings(uint8_t portID, uint16_t maxWait );					   //Returns the current protocol bits in the UBX-CFG-PRT command for a given port

	bool setI2COutput(uint8_t comSettings, uint16_t maxWait );				//Configure I2C port to output UBX, NMEA, RTCM3 or a combination thereof
	bool setUART1Output(uint8_t comSettings, uint16_t maxWait ); //Configure UART1 port to output UBX, NMEA, RTCM3 or a combination thereof
	bool setUART2Output(uint8_t comSettings, uint16_t maxWait ); //Configure UART2 port to output UBX, NMEA, RTCM3 or a combination thereof
	bool setUSBOutput(uint8_t comSettings, uint16_t maxWait );				//Configure USB port to output UBX, NMEA, RTCM3 or a combination thereof
	bool setSPIOutput(uint8_t comSettings, uint16_t maxWait );				//Configure SPI port to output UBX, NMEA, RTCM3 or a combination thereof

	//Functions to turn on/off message types for a given port ID (see COM_PORT_I2C, etc above)
	bool configureMessage(uint8_t msgClass, uint8_t msgID, uint8_t portID, uint8_t sendRate, uint16_t maxWait );
	bool enableMessage(uint8_t msgClass, uint8_t msgID, uint8_t portID, uint8_t sendRate , uint16_t maxWait );
	bool disableMessage(uint8_t msgClass, uint8_t msgID, uint8_t portID, uint16_t maxWait );
	bool enableNMEAMessage(uint8_t msgID, uint8_t portID, uint8_t sendRate , uint16_t maxWait );
	bool disableNMEAMessage(uint8_t msgID, uint8_t portID, uint16_t maxWait );
	bool enableRTCMmessage(uint8_t messageNumber, uint8_t portID, uint8_t sendRate, uint16_t maxWait ); //Given a message number turns on a message ID for output over given PortID
	bool disableRTCMmessage(uint8_t messageNumber, uint8_t portID, uint16_t maxWait );				   //Turn off given RTCM message from a given port

	//General configuration (used only on protocol v27 and higher - ie, ZED-F9P)
	//It is probably safe to assume that users of the ZED-F9P will be using I2C / Qwiic.
	//If they are using Serial then the higher baud rate will also help. So let's leave maxWait set to 250ms.
	uint8_t getVal8(uint16_t group, uint16_t id, uint8_t size, uint8_t layer , uint16_t maxWait ); //Returns the value at a given group/id/size location
	uint8_t setVal(uint32_t keyID, uint16_t value, uint8_t layer , uint16_t maxWait );			   //Sets the 16-bit value at a given group/id/size location
	uint8_t setVal8(uint32_t keyID, uint8_t value, uint8_t layer , uint16_t maxWait );			   //Sets the 8-bit value at a given group/id/size location
	uint8_t setVal16(uint32_t keyID, uint16_t value, uint8_t layer , uint16_t maxWait );		   //Sets the 16-bit value at a given group/id/size location
	uint8_t setVal32(uint32_t keyID, uint32_t value, uint8_t layer , uint16_t maxWait );		   //Sets the 32-bit value at a given group/id/size location
	uint8_t newCfgValset8(uint32_t keyID, uint8_t value, uint8_t layer );							   //Define a new UBX-CFG-VALSET with the given KeyID and 8-bit value
	uint8_t newCfgValset16(uint32_t keyID, uint16_t value, uint8_t layer );							   //Define a new UBX-CFG-VALSET with the given KeyID and 16-bit value
	uint8_t newCfgValset32(uint32_t keyID, uint32_t value, uint8_t layer );							   //Define a new UBX-CFG-VALSET with the given KeyID and 32-bit value
	uint8_t addCfgValset8(uint32_t keyID, uint8_t value);															   //Add a new KeyID and 8-bit value to an existing UBX-CFG-VALSET ubxPacket
	uint8_t addCfgValset16(uint32_t keyID, uint16_t value);															   //Add a new KeyID and 16-bit value to an existing UBX-CFG-VALSET ubxPacket
	uint8_t addCfgValset32(uint32_t keyID, uint32_t value);															   //Add a new KeyID and 32-bit value to an existing UBX-CFG-VALSET ubxPacket
	uint8_t sendCfgValset8(uint32_t keyID, uint8_t value, uint16_t maxWait );									   //Add the final KeyID and 8-bit value to an existing UBX-CFG-VALSET ubxPacket and send it
	uint8_t sendCfgValset16(uint32_t keyID, uint16_t value, uint16_t maxWait );								   //Add the final KeyID and 16-bit value to an existing UBX-CFG-VALSET ubxPacket and send it
	uint8_t sendCfgValset32(uint32_t keyID, uint32_t value, uint16_t maxWait );								   //Add the final KeyID and 32-bit value to an existing UBX-CFG-VALSET ubxPacket and send it

	//Functions used for RTK and base station setup
	//It is probably safe to assume that users of the RTK will be using I2C / Qwiic. So let's leave maxWait set to 250ms.
	bool getSurveyMode(uint16_t maxWait );																   //Get the current TimeMode3 settings
	bool setSurveyMode(uint8_t mode, uint16_t observationTime, float requiredAccuracy, uint16_t maxWait ); //Control survey in mode
	bool enableSurveyMode(uint16_t observationTime, float requiredAccuracy, uint16_t maxWait );			   //Begin Survey-In for NEO-M8P
	bool disableSurveyMode(uint16_t maxWait );															   //Stop Survey-In mode

	bool getSurveyStatus(uint16_t maxWait); //Reads survey in status and sets the global variables

	uint32_t getPositionAccuracy(uint16_t maxWait ); //Returns the 3D accuracy of the current high-precision fix, in mm. Supported on NEO-M8P, ZED-F9P,

	uint8_t getProtocolVersionHigh(uint16_t maxWait ); //Returns the PROTVER XX.00 from UBX-MON-VER register
	uint8_t getProtocolVersionLow(uint16_t maxWait );	//Returns the PROTVER 00.XX from UBX-MON-VER register
	bool getProtocolVersion(uint16_t maxWait );		//Queries module, loads low/high bytes

	bool getRELPOSNED(uint16_t maxWait ); //Get Relative Positioning Information of the NED frame

	//void enableDebugging(Stream &debugPort, bool printLimitedDebug);  //Given a port to print to, enable debug messages. Default to all, not limited.
	void disableDebugging(void);					   //Turn off debug statements
	void debugPrint(char *message);					   //Safely print debug statements
	void debugPrintln(char *message);				   //Safely print debug statements
	const char *statusString(sfe_ublox_status_e stat); //Pretty print the return value


	bool powerSaveMode(bool power_save, uint16_t maxWait );
	uint8_t getPowerSaveMode(uint16_t maxWait ); // Returns 255 if the sendCommand fails

	//Change the dynamic platform model using UBX-CFG-NAV5
	bool setDynamicModel(dynModel newDynamicModel, uint16_t maxWait );
	uint8_t getDynamicModel(uint16_t maxWait ); // Get the dynamic model - returns 255 if the sendCommand fails

	bool getEsfInfo(uint16_t maxWait);
	bool getEsfIns(uint16_t maxWait);
	bool getEsfDataInfo(uint16_t maxWait);
	bool getEsfRawDataInfo(uint16_t maxWait );
	sfe_ublox_status_e getSensState(uint8_t sensor, uint16_t maxWait );
	bool getVehAtt(uint16_t maxWait );
	
	// Change constellations
	bool setGPS_constellation_only(uint16_t maxWait);

  // Set power save config for the pico tracker lora
	bool set_powersave_config(uint16_t maxWait);

  // toggle putting into power save mode and continueous mode
	bool put_in_power_save_mode(uint16_t maxWait);
	bool put_in_continueous_mode(uint16_t maxWait);

	//Survey-in specific controls
	static struct svinStructure
	{
		bool active;
		bool valid;
		uint16_t observationTime;
		float meanAccuracy;
	} svin;

	//Relative Positioning Info in NED frame specific controls
	static struct frelPosInfoStructure
	{
		uint16_t refStationID;

		float relPosN;
		float relPosE;
		float relPosD;

		long relPosLength;
		long relPosHeading;

		int8_t relPosHPN;
		int8_t relPosHPE;
		int8_t relPosHPD;
		int8_t relPosHPLength;

		float accN;
		float accE;
		float accD;

		bool gnssFixOk;
		bool diffSoln;
		bool relPosValid;
		uint8_t carrSoln;
		bool isMoving;
		bool refPosMiss;
		bool refObsMiss;
	} relPosInfo;

	//The major datums we want to globally store
	static uint16_t gpsYear;
	static uint8_t gpsMonth;
	static uint8_t gpsDay;
	static uint8_t gpsHour;
	static uint8_t gpsMinute;
	static uint8_t gpsSecond;
	static uint16_t gpsMillisecond;
	static int32_t gpsNanosecond;
	static bool gpsDateValid;
	static bool gpsTimeValid;

	static int32_t latitude;		 //Degrees * 10^-7 (more accurate than floats)
	static int32_t longitude;		 //Degrees * 10^-7 (more accurate than floats)
	static int32_t altitude;		 //Number of mm above ellipsoid
	static int32_t altitudeMSL;	 //Number of mm above Mean Sea Level
	static uint8_t SIV;			 //Number of satellites used in position solution
	static uint8_t fixType;		 //Tells us when we have a solution aka lock
	static uint8_t gnssFixOK;  //Tells us whether fix is OK
	static uint8_t carrierSolution; //Tells us when we have an RTK float/fixed solution
	static int32_t groundSpeed;	 //mm/s
	static int32_t headingOfMotion; //degrees * 10^-5
	static uint16_t pDOP;			 //Positional dilution of precision
	static uint8_t versionLow;		 //Loaded from getProtocolVersion().
	static uint8_t versionHigh;

	static uint32_t timeOfWeek;		 // ms
	static int32_t highResLatitude;	 // Degrees * 10^-7
	static int32_t highResLongitude;	 // Degrees * 10^-7
	static int32_t elipsoid;			 // Height above ellipsoid in mm (Typo! Should be eLLipsoid! **Uncorrected for backward-compatibility.**)
	static int32_t meanSeaLevel;		 // Height above mean sea level in mm
	static int32_t geoidSeparation;	 // This seems to only be provided in NMEA GGA and GNS messages
	static uint32_t horizontalAccuracy; // mm * 10^-1 (i.e. 0.1mm)
	static uint32_t verticalAccuracy;	 // mm * 10^-1 (i.e. 0.1mm)
	static int8_t elipsoidHp;			 // High precision component of the height above ellipsoid in mm * 10^-1 (Deliberate typo! Should be eLLipsoidHp!)
	static int8_t meanSeaLevelHp;		 // High precision component of Height above mean sea level in mm * 10^-1
	static int8_t highResLatitudeHp;	 // High precision component of latitude: Degrees * 10^-9
	static int8_t highResLongitudeHp;	 // High precision component of longitude: Degrees * 10^-9

	static uint16_t rtcmFrameCounter = 0; //Tracks the type of incoming byte inside RTCM frame

#define DEF_NUM_SENS 7
	static struct deadReckData
	{
		uint8_t version;
		uint8_t fusionMode;

		uint8_t xAngRateVald;
		uint8_t yAngRateVald;
		uint8_t zAngRateVald;
		uint8_t xAccelVald;
		uint8_t yAccelVald;
		uint8_t zAccelVald;

		int32_t xAngRate;
		int32_t yAngRate;
		int32_t zAngRate;

		int32_t xAccel;
		int32_t yAccel;
		int32_t zAccel;

		// The array size is based on testing directly on M8U and F9R
		uint32_t rawData;
		uint32_t rawDataType;
		uint32_t rawTStamp;

		uint32_t data[DEF_NUM_SENS];
		uint32_t dataType[DEF_NUM_SENS];
		uint32_t dataTStamp[DEF_NUM_SENS];
	} imuMeas;

	static struct indivImuData
	{

		uint8_t numSens;

		uint8_t senType;
		bool isUsed;
		bool isReady;
		uint8_t calibStatus;
		uint8_t timeStatus;

		uint8_t freq; // Hz

		bool badMeas;
		bool badTag;
		bool missMeas;
		bool noisyMeas;
	} ubloxSen;

	static struct vehicleAttitude
	{
		// All values in degrees
		int32_t roll;
		int32_t pitch;
		int32_t heading;
		uint32_t accRoll;
		uint32_t accPitch;
		uint32_t accHeading;
	} vehAtt;


	//Depending on the sentence type the processor will load characters into different arrays
	static enum SentenceTypes
	{
		NONE = 0,
		NMEA,
		UBX,
		RTCM
	} currentSentence = NONE;

	//Depending on the ubx binary response class, store binary responses into different places
	static enum classTypes
	{
		CLASS_NONE = 0,
		CLASS_ACK,
		CLASS_NOT_AN_ACK
	} ubxFrameClass = CLASS_NONE;

	static enum commTypes
	{
		COMM_TYPE_I2C = 0,
		COMM_TYPE_SERIAL,
		COMM_TYPE_SPI
	} commType = COMM_TYPE_I2C; //Controls which port we look to for incoming bytes

	//Functions
	bool checkUbloxInternal(ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID); //Checks module with user selected commType
	uint32_t extractLong(uint8_t spotToStart);																	 //Combine four bytes from payload into long
	uint16_t extractInt(uint8_t spotToStart);																	 //Combine two bytes from payload into int
	uint8_t extractByte(uint8_t spotToStart);																	 //Get byte from payload
	int8_t extractSignedChar(uint8_t spotToStart);																 //Get signed 8-bit value from payload
	void addToChecksum(uint8_t incoming);																		 //Given an incoming byte, adjust rollingChecksumA/B



	//Variables
//	 TwoWire _i2cPort;				//The generic connection to user's chosen I2C hardware
//	Stream *_serialPort;			//The generic connection to user's chosen Serial hardware
//	Stream *_nmeaOutputPort = NULL; //The user can assign an output port to print NMEA sentences if they wish
//	Stream *_debugSerial;			//The stream to send debug messages to if enabled

	static uint8_t _gpsI2Caddress = 0x42; //Default 7-bit unshifted address of the ublox 6/7/8/M8/F9 series
	//This can be changed using the ublox configuration software

	static bool _printDebug = false; //Flag to print the serial commands we are sending to the Serial port for debug
	static bool _printLimitedDebug = false; //Flag to print limited debug messages. Useful for I2C debugging or high navigation rates

	//The packet buffers
	//These are pointed at from within the ubxPacket
	static uint8_t payloadAck[2];				  // Holds the requested ACK/NACK
	static uint8_t payloadCfg[MAX_PAYLOAD_SIZE]; // Holds the requested data packet
	static uint8_t payloadBuf[2];				  // Temporary buffer used to screen incoming packets or dump unrequested packets

	//Init the packet structures and init them with pointers to the payloadAck, payloadCfg and payloadBuf arrays
	static ubxPacket packetAck = {0, 0, 0, 0, 0, payloadAck, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED};
	static ubxPacket packetCfg = {0, 0, 0, 0, 0, payloadCfg, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED};
	static ubxPacket packetBuf = {0, 0, 0, 0, 0, payloadBuf, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED};

	//Flag if this packet is unrequested (and so should be ignored and not copied into packetCfg or packetAck)
	static bool ignoreThisPayload = false;

	//Identify which buffer is in use
	//Data is stored in packetBuf until the requested class and ID can be validated
	//If a match is seen, data is diverted into packetAck or packetCfg
	static sfe_ublox_packet_buffer_e activePacketBuffer = SFE_UBLOX_PACKET_PACKETBUF;

	//Limit checking of new data to every X ms
	//If we are expecting an update every X Hz then we should check every half that amount of time
	//Otherwise we may block ourselves from seeing new data
	static uint8_t i2cPollingWait = 100; //Default to 100ms. Adjusted when user calls setNavigationFrequency()

	static unsigned long lastCheck = 0;
	static bool autoPVT = false;			  //Whether autoPVT is enabled or not
	static bool autoPVTImplicitUpdate = true; // Whether autoPVT is triggered by accessing stale data (=true) or by a call to checkUblox (=false)
	static uint16_t ubxFrameCounter;			  //It counts all UBX frame. [Fixed header(2bytes), CLS(1byte), ID(1byte), length(2bytes), payload(x bytes), checksums(2bytes)]

	static uint8_t rollingChecksumA; //Rolls forward as we receive incoming bytes. Checked against the last two A/B checksum bytes
	static uint8_t rollingChecksumB; //Rolls forward as we receive incoming bytes. Checked against the last two A/B checksum bytes

	//Create bit field for staleness of each datum in PVT we want to monitor
	//moduleQueried.latitude goes true each time we call getPVT()
	//This reduces the number of times we have to call getPVT as this can take up to ~1s per read
	//depending on update rate
	static struct
	{
		uint32_t gpsiTOW : 1;
		uint32_t gpsYear : 1;
		uint32_t gpsMonth : 1;
		uint32_t gpsDay : 1;
		uint32_t gpsHour : 1;
		uint32_t gpsMinute : 1;
		uint32_t gpsSecond : 1;
		uint32_t gpsDateValid : 1;
		uint32_t gpsTimeValid : 1;
		uint32_t gpsNanosecond : 1;

		uint32_t all : 1;
		uint32_t longitude : 1;
		uint32_t latitude : 1;
		uint32_t altitude : 1;
		uint32_t altitudeMSL : 1;
		uint32_t SIV : 1;
		uint32_t fixType : 1;
		uint32_t gnssFixOK : 1;
		uint32_t carrierSolution : 1;
		uint32_t groundSpeed : 1;
		uint32_t headingOfMotion : 1;
		uint32_t pDOP : 1;
		uint32_t versionNumber : 1;
	} moduleQueried;

	static struct
	{
		uint16_t all : 1;
		uint16_t timeOfWeek : 1;
		uint16_t highResLatitude : 1;
		uint16_t highResLongitude : 1;
		uint16_t elipsoid : 1;
		uint16_t meanSeaLevel : 1;
		uint16_t geoidSeparation : 1; // Redundant but kept for backward-compatibility
		uint16_t horizontalAccuracy : 1;
		uint16_t verticalAccuracy : 1;
		uint16_t elipsoidHp : 1;
		uint16_t meanSeaLevelHp : 1;
		uint16_t highResLatitudeHp : 1;
		uint16_t highResLongitudeHp : 1;
	} highResModuleQueried;

	static uint16_t rtcmLen = 0;


#endif
#ifdef __cplusplus
}
#endif


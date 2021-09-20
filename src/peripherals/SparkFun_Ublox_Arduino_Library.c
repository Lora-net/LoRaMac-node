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

#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "i2c.h"
#include "SparkFun_Ublox_Arduino_Library.h"
#include <systime.h>
#include "delay.h"

extern I2c_t I2c;

const char *statusString(sfe_ublox_status_e stat)
{
  switch (stat)
  {
  case SFE_UBLOX_STATUS_SUCCESS:
    return "Success";
  case SFE_UBLOX_STATUS_FAIL:
    return "General Failure";
  case SFE_UBLOX_STATUS_CRC_FAIL:
    return "CRC Fail";
  case SFE_UBLOX_STATUS_TIMEOUT:
    return "Timeout";
  case SFE_UBLOX_STATUS_COMMAND_NACK:
    return "Command not acknowledged (NACK)";
  case SFE_UBLOX_STATUS_OUT_OF_RANGE:
    return "Out of range";
  case SFE_UBLOX_STATUS_INVALID_ARG:
    return "Invalid Arg";
  case SFE_UBLOX_STATUS_INVALID_OPERATION:
    return "Invalid operation";
  case SFE_UBLOX_STATUS_MEM_ERR:
    return "Memory Error";
  case SFE_UBLOX_STATUS_HW_ERR:
    return "Hardware Error";
  case SFE_UBLOX_STATUS_DATA_SENT:
    return "Data Sent";
  case SFE_UBLOX_STATUS_DATA_RECEIVED:
    return "Data Received";
  case SFE_UBLOX_STATUS_I2C_COMM_FAILURE:
    return "I2C Comm Failure";
  case SFE_UBLOX_STATUS_DATA_OVERWRITTEN:
    return "Data Packet Overwritten";
  default:
    return "Unknown Status";
  }
  //return "None";
}

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

static int32_t latitude;        //Degrees * 10^-7 (more accurate than floats)
static int32_t longitude;       //Degrees * 10^-7 (more accurate than floats)
static int32_t altitude;        //Number of mm above ellipsoid
static int32_t altitudeMSL;     //Number of mm above Mean Sea Level
static uint8_t SIV;             //Number of satellites used in position solution
static uint8_t fixType;         //Tells us when we have a solution aka lock
static uint8_t gnssFixOK;       //Tells us whether fix is OK
static uint8_t carrierSolution; //Tells us when we have an RTK float/fixed solution
static int32_t groundSpeed;     //mm/s
static int32_t headingOfMotion; //degrees * 10^-5
static uint16_t pDOP;           //Positional dilution of precision

static uint8_t _gpsI2Caddress = 0x42; //Default 7-bit unshifted address of the ublox 6/7/8/M8/F9 series
//This can be changed using the ublox configuration software

static bool _printDebug = false;        //Flag to print the serial commands we are sending to the Serial port for debug
static bool _printLimitedDebug = false; //Flag to print limited debug messages. Useful for I2C debugging or high navigation rates

//The packet buffers
//These are pointed at from within the ubxPacket
static uint8_t payloadAck[2];                // Holds the requested ACK/NACK
static uint8_t payloadCfg[MAX_PAYLOAD_SIZE]; // Holds the requested data packet
static uint8_t payloadBuf[2];                // Temporary buffer used to screen incoming packets or dump unrequested packets

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
static bool autoPVT = false;              //Whether autoPVT is enabled or not
static bool autoPVTImplicitUpdate = true; // Whether autoPVT is triggered by accessing stale data (=true) or by a call to checkUblox (=false)
static uint16_t ubxFrameCounter;          //It counts all UBX frame. [Fixed header(2bytes), CLS(1byte), ID(1byte), length(2bytes), payload(x bytes), checksums(2bytes)]

static uint8_t rollingChecksumA; //Rolls forward as we receive incoming bytes. Checked against the last two A/B checksum bytes
static uint8_t rollingChecksumB; //Rolls forward as we receive incoming bytes. Checked against the last two A/B checksum bytes

static uint32_t timeOfWeek;         // ms
static int32_t highResLatitude;     // Degrees * 10^-7
static int32_t highResLongitude;    // Degrees * 10^-7
static int32_t elipsoid;            // Height above ellipsoid in mm (Typo! Should be eLLipsoid! **Uncorrected for backward-compatibility.**)
static int32_t meanSeaLevel;        // Height above mean sea level in mm
static uint32_t horizontalAccuracy; // mm * 10^-1 (i.e. 0.1mm)
static uint32_t verticalAccuracy;   // mm * 10^-1 (i.e. 0.1mm)
static int8_t elipsoidHp;           // High precision component of the height above ellipsoid in mm * 10^-1 (Deliberate typo! Should be eLLipsoidHp!)
static int8_t meanSeaLevelHp;       // High precision component of Height above mean sea level in mm * 10^-1
static int8_t highResLatitudeHp;    // High precision component of latitude: Degrees * 10^-9
static int8_t highResLongitudeHp;   // High precision component of longitude: Degrees * 10^-9

static uint16_t rtcmFrameCounter = 0; //Tracks the type of incoming byte inside RTCM frame

//Depending on the sentence type the processor will load characters into different arrays
static enum SentenceTypes { NONE = 0,
                            NMEA,
                            UBX,
                            RTCM
} currentSentence = NONE;

static enum commTypes { COMM_TYPE_I2C = 0,
                        COMM_TYPE_SERIAL,
                        COMM_TYPE_SPI
} commType = COMM_TYPE_I2C; //Controls which port we look to for incoming bytes

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

//Called regularly to check for available bytes on the user' specified port
bool checkUblox(uint8_t requestedClass, uint8_t requestedID)
{
  return checkUbloxInternal(&packetCfg, requestedClass, requestedID);
}

//Called regularly to check for available bytes on the user' specified port
bool checkUbloxInternal(ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID)
{
  if (commType == COMM_TYPE_I2C)
    return (checkUbloxI2C(incomingUBX, requestedClass, requestedID));

  return false;
}

//Polls I2C for data, passing any new bytes to process()
//Returns true if new bytes are available
bool checkUbloxI2C(ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID)
{
  if (SysTimeToMs(SysTimeGet()) - lastCheck >= i2cPollingWait)
  {
    //Get the number of bytes available from the module
    uint16_t bytesAvailable = 0;

    uint8_t buff_rx[2] = {0};

    if (I2cReadMemBuffer(&I2c, (uint16_t)_gpsI2Caddress << 1, (uint16_t)0xFD, buff_rx, 2) != LMN_STATUS_OK)
    {
      return (false); //Sensor did not ACK
    }

    uint8_t msb = buff_rx[0];
    uint8_t lsb = buff_rx[1];

    if (lsb == 0xFF)
    {
      //I believe this is a Ublox bug. Device should never present an 0xFF.
      if ((_printDebug == true) || (_printLimitedDebug == true)) // printf this if doing limited debugging
      {
        printf("checkUbloxI2C: Ublox bug, length lsb is 0xFF\r\n");
      }

      lastCheck = SysTimeToMs(SysTimeGet()); //Put off checking to avoid I2C bus traffic
      return (false);
    }
    bytesAvailable = (uint16_t)msb << 8 | lsb;

    if (bytesAvailable == 0)
    {
      if (_printDebug == true)
      {
        printf("checkUbloxI2C: OK, zero bytes available\r\n");
      }
      lastCheck = SysTimeToMs(SysTimeGet()); //Put off checking to avoid I2C bus traffic
      return (false);
    }

    //Check for undocumented bit error. We found this doing logic scans.
    //This error is rare but if we incorrectly interpret the first bit of the two 'data available' bytes as 1
    //then we have far too many bytes to check. May be related to I2C setup time violations: https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library/issues/40
    if (bytesAvailable & ((uint16_t)1 << 15))
    {
      //Clear the MSbit
      bytesAvailable &= ~((uint16_t)1 << 15);

      if ((_printDebug == true) || (_printLimitedDebug == true)) // printf this if doing limited debugging
      {
        printf("checkUbloxI2C: Bytes available error:");
        printf("%d\r\n", bytesAvailable);
      }
    }

    if (bytesAvailable > 100)
    {
      if (_printDebug == true)
      {
        printf("checkUbloxI2C: Large packet of ");
        printf("%d", bytesAvailable);
        printf(" bytes received\r\n");
      }
    }
    else
    {
      if (_printDebug == true)
      {
        printf("checkUbloxI2C: Reading ");
        printf("%d", bytesAvailable);
        printf(" bytes\r\n");
      }
    }

    if (I2cReadMemBuffer(&I2c, (uint16_t)_gpsI2Caddress << 1, (uint16_t)0xFF, payloadCfg, bytesAvailable) != LMN_STATUS_OK)
    {
      return (false); //Sensor did not ACK
    }
    for (int i = 0; i < bytesAvailable; i++)
    {
      uint8_t incoming = payloadCfg[i];
      process(incoming, incomingUBX, requestedClass, requestedID); //Process this valid character
    }
  }

  return (true);
}

//Processes NMEA and UBX binary sentences one byte at a time
//Take a given byte and file it into the proper array
void process(uint8_t incoming, ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID)
{
  if ((currentSentence == NONE) || (currentSentence == NMEA))
  {
    if (incoming == 0xB5) //UBX binary frames start with 0xB5, aka μ
    {
      //This is the start of a binary sentence. Reset flags.
      //We still don't know the response class
      ubxFrameCounter = 0;
      currentSentence = UBX;
      //Reset the packetBuf.counter even though we will need to reset it again when ubxFrameCounter == 2
      packetBuf.counter = 0;
      ignoreThisPayload = false; //We should not ignore this payload - yet
      //Store data in packetBuf until we know if we have a requested class and ID match
      activePacketBuffer = SFE_UBLOX_PACKET_PACKETBUF;
    }
    else if (incoming == '$')
    {
      currentSentence = NMEA;
    }
    else if (incoming == 0xD3) //RTCM frames start with 0xD3
    {
      rtcmFrameCounter = 0;
      currentSentence = RTCM;
    }
    else
    {
      //This character is unknown or we missed the previous start of a sentence
    }
  }

  //Depending on the sentence, pass the character to the individual processor
  if (currentSentence == UBX)
  {
    //Decide what type of response this is
    if ((ubxFrameCounter == 0) && (incoming != 0xB5))      //ISO 'μ'
      currentSentence = NONE;                              //Something went wrong. Reset.
    else if ((ubxFrameCounter == 1) && (incoming != 0x62)) //ASCII 'b'
      currentSentence = NONE;                              //Something went wrong. Reset.
    // Note to future self:
    // There may be some duplication / redundancy in the next few lines as processUBX will also
    // load information into packetBuf, but we'll do it here too for clarity
    else if (ubxFrameCounter == 2) //Class
    {
      // Record the class in packetBuf until we know what to do with it
      packetBuf.cls = incoming; // (Duplication)
      rollingChecksumA = 0;     //Reset our rolling checksums here (not when we receive the 0xB5)
      rollingChecksumB = 0;
      packetBuf.counter = 0;                                   //Reset the packetBuf.counter (again)
      packetBuf.valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED; // Reset the packet validity (redundant?)
      packetBuf.startingSpot = incomingUBX->startingSpot;      //Copy the startingSpot
    }
    else if (ubxFrameCounter == 3) //ID
    {
      // Record the ID in packetBuf until we know what to do with it
      packetBuf.id = incoming; // (Duplication)
      //We can now identify the type of response
      //If the packet we are receiving is not an ACK then check for a class and ID match
      if (packetBuf.cls != UBX_CLASS_ACK)
      {
        //This is not an ACK so check for a class and ID match
        if ((packetBuf.cls == requestedClass) && (packetBuf.id == requestedID))
        {
          //This is not an ACK and we have a class and ID match
          //So start diverting data into incomingUBX (usually packetCfg)
          activePacketBuffer = SFE_UBLOX_PACKET_PACKETCFG;
          incomingUBX->cls = packetBuf.cls; //Copy the class and ID into incomingUBX (usually packetCfg)
          incomingUBX->id = packetBuf.id;
          incomingUBX->counter = packetBuf.counter; //Copy over the .counter too
        }
        else
        {
          //This is not an ACK and we do not have a class and ID match
          //so we should keep diverting data into packetBuf and ignore the payload
          ignoreThisPayload = true;
        }
      }
      else
      {
        // This is an ACK so it is to early to do anything with it
        // We need to wait until we have received the length and data bytes
        // So we should keep diverting data into packetBuf
      }
    }
    else if (ubxFrameCounter == 4) //Length LSB
    {
      //We should save the length in packetBuf even if activePacketBuffer == SFE_UBLOX_PACKET_PACKETCFG
      packetBuf.len = incoming; // (Duplication)
    }
    else if (ubxFrameCounter == 5) //Length MSB
    {
      //We should save the length in packetBuf even if activePacketBuffer == SFE_UBLOX_PACKET_PACKETCFG
      packetBuf.len |= incoming << 8; // (Duplication)
    }
    else if (ubxFrameCounter == 6) //This should be the first byte of the payload unless .len is zero
    {
      if (packetBuf.len == 0) // Check if length is zero (hopefully this is impossible!)
      {
        if (_printDebug == true)
        {
          printf("process: ZERO LENGTH packet received: Class: 0x");
          printf("%02X", packetBuf.cls);
          printf(" ID: 0x");
          printf("%02X\r\n", packetBuf.id);
        }
        //If length is zero (!) this will be the first byte of the checksum so record it
        packetBuf.checksumA = incoming;
      }
      else
      {
        //The length is not zero so record this byte in the payload
        packetBuf.payload[0] = incoming;
      }
    }
    else if (ubxFrameCounter == 7) //This should be the second byte of the payload unless .len is zero or one
    {
      if (packetBuf.len == 0) // Check if length is zero (hopefully this is impossible!)
      {
        //If length is zero (!) this will be the second byte of the checksum so record it
        packetBuf.checksumB = incoming;
      }
      else if (packetBuf.len == 1) // Check if length is one
      {
        //The length is one so this is the first byte of the checksum
        packetBuf.checksumA = incoming;
      }
      else // Length is >= 2 so this must be a payload byte
      {
        packetBuf.payload[1] = incoming;
      }
      // Now that we have received two payload bytes, we can check for a matching ACK/NACK
      if ((activePacketBuffer == SFE_UBLOX_PACKET_PACKETBUF) // If we are not already processing a data packet
          && (packetBuf.cls == UBX_CLASS_ACK)                // and if this is an ACK/NACK
          && (packetBuf.payload[0] == requestedClass)        // and if the class matches
          && (packetBuf.payload[1] == requestedID))          // and if the ID matches
      {
        if (packetBuf.len == 2) // Check if .len is 2
        {
          // Then this is a matching ACK so copy it into packetAck
          activePacketBuffer = SFE_UBLOX_PACKET_PACKETACK;
          packetAck.cls = packetBuf.cls;
          packetAck.id = packetBuf.id;
          packetAck.len = packetBuf.len;
          packetAck.counter = packetBuf.counter;
          packetAck.payload[0] = packetBuf.payload[0];
          packetAck.payload[1] = packetBuf.payload[1];
        }
        else // Length is not 2 (hopefully this is impossible!)
        {
          if (_printDebug == true)
          {
            printf("process: ACK received with .len != 2: Class: 0x");
            printf("%02X ", packetBuf.payload[0]);
            printf(" ID: 0x");
            printf("%02X ", packetBuf.payload[1]);
            printf(" len: ");
            printf("%02X\r\n", packetBuf.len);
          }
        }
      }
    }

    //Divert incoming into the correct buffer
    if (activePacketBuffer == SFE_UBLOX_PACKET_PACKETACK)
      processUBX(incoming, &packetAck, requestedClass, requestedID);
    else if (activePacketBuffer == SFE_UBLOX_PACKET_PACKETCFG)
      processUBX(incoming, incomingUBX, requestedClass, requestedID);
    else // if (activePacketBuffer == SFE_UBLOX_PACKET_PACKETBUF)
      processUBX(incoming, &packetBuf, requestedClass, requestedID);

    //Finally, increment the frame counter
    ubxFrameCounter++;
  }
  else if (currentSentence == NMEA)
  {
  }
  else if (currentSentence == RTCM)
  {
  }
}

//Given a character, file it away into the uxb packet structure
//Set valid to VALID or NOT_VALID once sentence is completely received and passes or fails CRC
//The payload portion of the packet can be 100s of bytes but the max array
//size is MAX_PAYLOAD_SIZE bytes. startingSpot can be set so we only record
//a subset of bytes within a larger packet.
void processUBX(uint8_t incoming, ubxPacket *incomingUBX, uint8_t requestedClass, uint8_t requestedID)
{
  //Add all incoming bytes to the rolling checksum
  //Stop at len+4 as this is the checksum bytes to that should not be added to the rolling checksum
  if (incomingUBX->counter < incomingUBX->len + 4)
    addToChecksum(incoming);

  if (incomingUBX->counter == 0)
  {
    incomingUBX->cls = incoming;
  }
  else if (incomingUBX->counter == 1)
  {
    incomingUBX->id = incoming;
  }
  else if (incomingUBX->counter == 2) //Len LSB
  {
    incomingUBX->len = incoming;
  }
  else if (incomingUBX->counter == 3) //Len MSB
  {
    incomingUBX->len |= incoming << 8;
  }
  else if (incomingUBX->counter == incomingUBX->len + 4) //ChecksumA
  {
    incomingUBX->checksumA = incoming;
  }
  else if (incomingUBX->counter == incomingUBX->len + 5) //ChecksumB
  {
    incomingUBX->checksumB = incoming;

    currentSentence = NONE; //We're done! Reset the sentence to being looking for a new start char

    //Validate this sentence
    if ((incomingUBX->checksumA == rollingChecksumA) && (incomingUBX->checksumB == rollingChecksumB))
    {
      incomingUBX->valid = SFE_UBLOX_PACKET_VALIDITY_VALID; // Flag the packet as valid

      // Let's check if the class and ID match the requestedClass and requestedID
      // Remember - this could be a data packet or an ACK packet
      if ((incomingUBX->cls == requestedClass) && (incomingUBX->id == requestedID))
      {
        incomingUBX->classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_VALID; // If we have a match, set the classAndIDmatch flag to valid
      }

      // If this is an ACK then let's check if the class and ID match the requestedClass and requestedID
      else if ((incomingUBX->cls == UBX_CLASS_ACK) && (incomingUBX->id == UBX_ACK_ACK) && (incomingUBX->payload[0] == requestedClass) && (incomingUBX->payload[1] == requestedID))
      {
        incomingUBX->classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_VALID; // If we have a match, set the classAndIDmatch flag to valid
      }

      // If this is a NACK then let's check if the class and ID match the requestedClass and requestedID
      else if ((incomingUBX->cls == UBX_CLASS_ACK) && (incomingUBX->id == UBX_ACK_NACK) && (incomingUBX->payload[0] == requestedClass) && (incomingUBX->payload[1] == requestedID))
      {
        incomingUBX->classAndIDmatch = SFE_UBLOX_PACKET_NOTACKNOWLEDGED; // If we have a match, set the classAndIDmatch flag to NOTACKNOWLEDGED
        if (_printDebug == true)
        {
          printf("processUBX: NACK received: Requested Class: 0x");
          printf("%02X ", incomingUBX->payload[0]);
          printf(" Requested ID: 0x");
          printf("%02X \r\n", incomingUBX->payload[1]);
        }
      }

      if (_printDebug == true)
      {
        printf("Incoming: Size: ");
        printf("%d", incomingUBX->len);
        printf(" Received: ");
        printPacket(incomingUBX);

        if (incomingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID)
        {
          printf("packetCfg now valid\r\n");
        }
        if (packetAck.valid == SFE_UBLOX_PACKET_VALIDITY_VALID)
        {
          printf("packetAck now valid\r\n");
        }
        if (incomingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID)
        {
          printf("packetCfg classAndIDmatch\r\n");
        }
        if (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID)
        {
          printf("packetAck classAndIDmatch\r\n");
        }
      }

      //We've got a valid packet, now do something with it but only if ignoreThisPayload is false
      if (ignoreThisPayload == false)
      {
        processUBXpacket(incomingUBX);
      }
    }
    else // Checksum failure
    {
      incomingUBX->valid = SFE_UBLOX_PACKET_VALIDITY_NOT_VALID;

      // Let's check if the class and ID match the requestedClass and requestedID.
      // This is potentially risky as we are saying that we saw the requested Class and ID
      // but that the packet checksum failed. Potentially it could be the class or ID bytes
      // that caused the checksum error!
      if ((incomingUBX->cls == requestedClass) && (incomingUBX->id == requestedID))
      {
        incomingUBX->classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_VALID; // If we have a match, set the classAndIDmatch flag to not valid
      }
      // If this is an ACK then let's check if the class and ID match the requestedClass and requestedID
      else if ((incomingUBX->cls == UBX_CLASS_ACK) && (incomingUBX->payload[0] == requestedClass) && (incomingUBX->payload[1] == requestedID))
      {
        incomingUBX->classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_VALID; // If we have a match, set the classAndIDmatch flag to not valid
      }

      if ((_printDebug == true) || (_printLimitedDebug == true)) // printf this if doing limited debugging
      {

        printf("Checksum failed:");
        printf(" checksumA: ");
        printf("%d", incomingUBX->checksumA);
        printf(" checksumB: ");
        printf("%d", incomingUBX->checksumB);

        printf(" rollingChecksumA: ");
        printf("%d", rollingChecksumA);
        printf(" rollingChecksumB: ");
        printf("%d", rollingChecksumB);
        printf("\r\n");

        printf("Failed  : ");
        printf("Size: ");
        printf("%d", incomingUBX->len);
        printf(" Received: ");
        printPacket(incomingUBX);
      }
    }
  }
  else //Load this byte into the payload array
  {
    //If a UBX_NAV_PVT packet comes in asynchronously, we need to fudge the startingSpot
    uint16_t startingSpot = incomingUBX->startingSpot;
    if (incomingUBX->cls == UBX_CLASS_NAV && incomingUBX->id == UBX_NAV_PVT)
      startingSpot = 0;
    //Begin recording if counter goes past startingSpot
    if ((incomingUBX->counter - 4) >= startingSpot)
    {
      //Check to see if we have room for this byte
      if (((incomingUBX->counter - 4) - startingSpot) < MAX_PAYLOAD_SIZE) //If counter = 208, starting spot = 200, we're good to record.
      {
        // Check if this is payload data which should be ignored
        if (ignoreThisPayload == false)
        {
          incomingUBX->payload[incomingUBX->counter - 4 - startingSpot] = incoming; //Store this byte into payload array
        }
      }
    }
  }

  //Increment the counter
  incomingUBX->counter++;

  if (incomingUBX->counter == MAX_PAYLOAD_SIZE)
  {
    //Something has gone very wrong
    currentSentence = NONE; //Reset the sentence to being looking for a new start char
    if (_printDebug == true)
    {
      printf("processUBX: counter hit MAX_PAYLOAD_SIZE");
    }
  }
}

//Once a packet has been received and validated, identify this packet's class/id and update internal flags
//Note: if the user requests a PVT or a HPPOSLLH message using a custom packet, the data extraction will
//      not work as expected beacuse extractLong etc are hardwired to packetCfg payloadCfg. Ideally
//      extractLong etc should be updated so they receive a pointer to the packet buffer.
void processUBXpacket(ubxPacket *msg)
{
  switch (msg->cls)
  {
  case UBX_CLASS_NAV:
    if (msg->id == UBX_NAV_PVT && msg->len == 92)
    {
      //Parse various byte fields into global vars
      int startingSpot = 0; //fixed value used in processUBX

      timeOfWeek = extractLong(0);
      gpsMillisecond = extractLong(0) % 1000; //Get last three digits of iTOW
      gpsYear = extractInt(4);
      gpsMonth = extractByte(6);
      gpsDay = extractByte(7);
      gpsHour = extractByte(8);
      gpsMinute = extractByte(9);
      gpsSecond = extractByte(10);
      gpsDateValid = extractByte(11) & 0x01;
      gpsTimeValid = (extractByte(11) & 0x02) >> 1;
      gpsNanosecond = extractLong(16); //Includes milliseconds

      fixType = extractByte(20 - startingSpot);
      gnssFixOK = (extractByte(21 - startingSpot) >> 0) & 0x01; // get the first bit(least significant bit)
      carrierSolution = extractByte(21 - startingSpot) >> 6;    //Get 6th&7th bits of this byte
      SIV = extractByte(23 - startingSpot);
      longitude = extractLong(24 - startingSpot);
      latitude = extractLong(28 - startingSpot);
      altitude = extractLong(32 - startingSpot);
      altitudeMSL = extractLong(36 - startingSpot);
      groundSpeed = extractLong(60 - startingSpot);
      headingOfMotion = extractLong(64 - startingSpot);
      pDOP = extractInt(76 - startingSpot);

      //Mark all datums as fresh (not read before)
      moduleQueried.gpsiTOW = true;
      moduleQueried.gpsYear = true;
      moduleQueried.gpsMonth = true;
      moduleQueried.gpsDay = true;
      moduleQueried.gpsHour = true;
      moduleQueried.gpsMinute = true;
      moduleQueried.gpsSecond = true;
      moduleQueried.gpsDateValid = true;
      moduleQueried.gpsTimeValid = true;
      moduleQueried.gpsNanosecond = true;

      moduleQueried.all = true;
      moduleQueried.longitude = true;
      moduleQueried.latitude = true;
      moduleQueried.altitude = true;
      moduleQueried.altitudeMSL = true;
      moduleQueried.SIV = true;
      moduleQueried.fixType = true;
      moduleQueried.gnssFixOK = true;
      moduleQueried.carrierSolution = true;
      moduleQueried.groundSpeed = true;
      moduleQueried.headingOfMotion = true;
      moduleQueried.pDOP = true;
    }
    else if (msg->id == UBX_NAV_HPPOSLLH && msg->len == 36)
    {
      timeOfWeek = extractLong(4);
      highResLongitude = extractLong(8);
      highResLatitude = extractLong(12);
      elipsoid = extractLong(16);
      meanSeaLevel = extractLong(20);
      highResLongitudeHp = extractSignedChar(24);
      highResLatitudeHp = extractSignedChar(25);
      elipsoidHp = extractSignedChar(26);
      meanSeaLevelHp = extractSignedChar(27);
      horizontalAccuracy = extractLong(28);
      verticalAccuracy = extractLong(32);

      highResModuleQueried.all = true;
      highResModuleQueried.highResLatitude = true;
      highResModuleQueried.highResLatitudeHp = true;
      highResModuleQueried.highResLongitude = true;
      highResModuleQueried.highResLongitudeHp = true;
      highResModuleQueried.elipsoid = true;
      highResModuleQueried.elipsoidHp = true;
      highResModuleQueried.meanSeaLevel = true;
      highResModuleQueried.meanSeaLevelHp = true;
      highResModuleQueried.geoidSeparation = true;
      highResModuleQueried.horizontalAccuracy = true;
      highResModuleQueried.verticalAccuracy = true;
      moduleQueried.gpsiTOW = true; // this can arrive via HPPOS too.

      if (_printDebug == true)
      {
        printf("Sec: ");
        printf("%3.5f\n", ((float)extractLong(4)) / 1000.0f);
        printf(" ");
        printf("LON: ");
        printf("%3.5f\n", ((float)(int32_t)extractLong(8)) / 10000000.0f);
        printf(" ");
        printf("LAT: ");
        printf("%3.5f\n", ((float)(int32_t)extractLong(12)) / 10000000.0f);
        printf(" ");
        printf("ELI M: ");
        printf("%3.5f\n", ((float)(int32_t)extractLong(16)) / 1000.0f);
        printf(" ");
        printf("MSL M: ");
        printf("%3.5f\n", ((float)(int32_t)extractLong(20)) / 1000.0f);
        printf(" ");
        printf("LON HP: ");
        printf("%d\r\n", extractSignedChar(24));
        printf(" ");
        printf("LAT HP: ");
        printf("%d\r\n", extractSignedChar(25));
        printf(" ");
        printf("ELI HP: ");
        printf("%d\r\n", extractSignedChar(26));
        printf(" ");
        printf("MSL HP: ");
        printf("%d\r\n", extractSignedChar(27));
        printf(" ");
        printf("HA 2D M: ");
        printf("%3.5f\n", ((float)(int32_t)extractLong(28)) / 10000.0f);
        printf(" ");
        printf("VERT M: ");
        printf("%3.5f\n", ((float)(int32_t)extractLong(32)) / 10000.0f);
      }
    }
    break;
  }
}

//Given a packet and payload, send everything including CRC bytes via I2C port
sfe_ublox_status_e sendCommand(ubxPacket *outgoingUBX, uint16_t maxWait)
{
  sfe_ublox_status_e retVal = SFE_UBLOX_STATUS_SUCCESS;

  calcChecksum(outgoingUBX); //Sets checksum A and B bytes of the packet

  if (_printDebug == true)
  {
    printf("\nSending: ");
    printPacket(outgoingUBX);
  }

  if (commType == COMM_TYPE_I2C)
  {
    retVal = sendI2cCommand(outgoingUBX, maxWait);
    if (retVal != SFE_UBLOX_STATUS_SUCCESS)
    {
      if (_printDebug == true)
      {
        printf("Send I2C Command failed");
      }
      return retVal;
    }
  }

  if (maxWait > 0)
  {
    //Depending on what we just sent, either we need to look for an ACK or not
    if (outgoingUBX->cls == UBX_CLASS_CFG)
    {
      if (_printDebug == true)
      {
        printf("sendCommand: Waiting for ACK response\r\n");
      }
      retVal = waitForACKResponse(outgoingUBX, outgoingUBX->cls, outgoingUBX->id, maxWait); //Wait for Ack response
    }
    else
    {
      if (_printDebug == true)
      {
        printf("sendCommand: Waiting for No ACK response\r\n");
      }
      retVal = waitForNoACKResponse(outgoingUBX, outgoingUBX->cls, outgoingUBX->id, maxWait); //Wait for Ack response
    }
  }
  return retVal;
}

//Returns false if sensor fails to respond to I2C traffic
sfe_ublox_status_e sendI2cCommand(ubxPacket *outgoingUBX, uint16_t maxWait)
{

  uint8_t cnt = 0;
  /* Total packet consists of 8 bytes minimum and then payload */
  uint8_t GPS_buffer[outgoingUBX->len + 8];
  GPS_buffer[cnt++] = UBX_SYNCH_1; //μ - oh ublox, you're funny. I will call you micro-blox from now on.
  GPS_buffer[cnt++] = UBX_SYNCH_2; //b
  GPS_buffer[cnt++] = outgoingUBX->cls;
  GPS_buffer[cnt++] = outgoingUBX->id;
  GPS_buffer[cnt++] = outgoingUBX->len & 0xFF; //LSB
  GPS_buffer[cnt++] = outgoingUBX->len >> 8;   //MSB

  for (uint16_t x = 0; x < outgoingUBX->len; x++)
  {
    GPS_buffer[cnt++] = outgoingUBX->payload[x];
  }

  GPS_buffer[cnt++] = outgoingUBX->checksumA;
  GPS_buffer[cnt++] = outgoingUBX->checksumB;

  return ((I2cWriteMemBuffer(&I2c, _gpsI2Caddress << 1, (uint16_t)0xFF, GPS_buffer, cnt) == LMN_STATUS_OK)
              ? SFE_UBLOX_STATUS_SUCCESS
              : SFE_UBLOX_STATUS_I2C_COMM_FAILURE);
}

//Given a message, calc and store the two byte "8-Bit Fletcher" checksum over the entirety of the message
//This is called before we send a command message
void calcChecksum(ubxPacket *msg)
{
  msg->checksumA = 0;
  msg->checksumB = 0;

  msg->checksumA += msg->cls;
  msg->checksumB += msg->checksumA;

  msg->checksumA += msg->id;
  msg->checksumB += msg->checksumA;

  msg->checksumA += (msg->len & 0xFF);
  msg->checksumB += msg->checksumA;

  msg->checksumA += (msg->len >> 8);
  msg->checksumB += msg->checksumA;

  for (uint16_t i = 0; i < msg->len; i++)
  {
    msg->checksumA += msg->payload[i];
    msg->checksumB += msg->checksumA;
  }
}

//Given a message and a byte, add to rolling "8-Bit Fletcher" checksum
//This is used when receiving messages from module
void addToChecksum(uint8_t incoming)
{
  rollingChecksumA += incoming;
  rollingChecksumB += rollingChecksumA;
}

//Pretty prints the current ubxPacket
void printPacket(ubxPacket *packet)
{
  if (_printDebug == true)
  {
    printf("CLS:");
    if (packet->cls == UBX_CLASS_NAV) //1
      printf("NAV");
    else if (packet->cls == UBX_CLASS_ACK) //5
      printf("ACK");
    else if (packet->cls == UBX_CLASS_CFG) //6
      printf("CFG");
    else if (packet->cls == UBX_CLASS_MON) //0x0A
      printf("MON");
    else
    {
      printf("0x");
      printf("%d", packet->cls);
    }

    printf(" ID:");
    if (packet->cls == UBX_CLASS_NAV && packet->id == UBX_NAV_PVT)
      printf("PVT");
    else if (packet->cls == UBX_CLASS_CFG && packet->id == UBX_CFG_RATE)
      printf("RATE");
    else if (packet->cls == UBX_CLASS_CFG && packet->id == UBX_CFG_CFG)
      printf("SAVE");
    else
    {
      printf("0x");
      printf("%02X", packet->id);
    }

    printf(" Len: 0x");
    printf("%02X", packet->len);

    // Only printf the payload is ignoreThisPayload is false otherwise
    // we could be printing gibberish from beyond the end of packetBuf
    if (ignoreThisPayload == false)
    {
      printf(" Payload:");

      for (int x = 0; x < packet->len; x++)
      {
        printf("0x");
        printf("%02X ", packet->payload[x]);
      }
    }
    else
    {
      printf(" Payload: IGNORED");
    }
    printf("\r\n");
  }
}

//It is entirely possible that our packetCfg and packetAck were received successfully
//but while we are still in the "if (checkUblox() == true)" loop a PVT packet is processed
//or _starts_ to arrive (remember that Serial data can arrive very slowly).

//Returns SFE_UBLOX_STATUS_DATA_RECEIVED if we got an ACK and a valid packetCfg (module is responding with register content)
//Returns SFE_UBLOX_STATUS_DATA_SENT if we got an ACK and no packetCfg (no valid packetCfg needed, module absorbs new register data)
//Returns SFE_UBLOX_STATUS_FAIL if something very bad happens (e.g. a double checksum failure)
//Returns SFE_UBLOX_STATUS_COMMAND_NACK if the packet was not-acknowledged (NACK)
//Returns SFE_UBLOX_STATUS_CRC_FAIL if we had a checksum failure
//Returns SFE_UBLOX_STATUS_TIMEOUT if we timed out
//Returns SFE_UBLOX_STATUS_DATA_OVERWRITTEN if we got an ACK and a valid packetCfg but that the packetCfg has been
// or is currently being overwritten (remember that Serial data can arrive very slowly)
sfe_ublox_status_e waitForACKResponse(ubxPacket *outgoingUBX, uint8_t requestedClass, uint8_t requestedID, uint16_t maxTime)
{
  outgoingUBX->valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED; //This will go VALID (or NOT_VALID) when we receive a response to the packet we sent
  packetAck.valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;
  packetBuf.valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;
  outgoingUBX->classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED; // This will go VALID (or NOT_VALID) when we receive a packet that matches the requested class and ID
  packetAck.classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;
  packetBuf.classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;

  unsigned long startTime = SysTimeToMs(SysTimeGet());
  while (SysTimeToMs(SysTimeGet()) - startTime < maxTime)
  {
    if (checkUbloxInternal(outgoingUBX, requestedClass, requestedID) == true) //See if new data is available. Process bytes as they come in.
    {
      // If both the outgoingUBX->classAndIDmatch and packetAck.classAndIDmatch are VALID
      // and outgoingUBX->valid is _still_ VALID and the class and ID _still_ match
      // then we can be confident that the data in outgoingUBX is valid
      if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->cls == requestedClass) && (outgoingUBX->id == requestedID))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: valid data and valid ACK received after ");
          printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec\r\n");
        }
        return (SFE_UBLOX_STATUS_DATA_RECEIVED); //We received valid data and a correct ACK!
      }

      // We can be confident that the data packet (if we are going to get one) will always arrive
      // before the matching ACK. So if we sent a config packet which only produces an ACK
      // then outgoingUBX->classAndIDmatch will be NOT_DEFINED and the packetAck.classAndIDmatch will VALID.
      // We should not check outgoingUBX->valid, outgoingUBX->cls or outgoingUBX->id
      // as these may have been changed by a PVT packet.
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: no data and valid ACK after ");
          printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec\r\n");
        }
        return (SFE_UBLOX_STATUS_DATA_SENT); //We got an ACK but no data...
      }

      // If both the outgoingUBX->classAndIDmatch and packetAck.classAndIDmatch are VALID
      // but the outgoingUBX->cls or ID no longer match then we can be confident that we had
      // valid data but it has been or is currently being overwritten by another packet (e.g. PVT).
      // If (e.g.) a PVT packet is _being_ received: outgoingUBX->valid will be NOT_DEFINED
      // If (e.g.) a PVT packet _has been_ received: outgoingUBX->valid will be VALID (or just possibly NOT_VALID)
      // So we cannot use outgoingUBX->valid as part of this check.
      // Note: the addition of packetBuf should make this check redundant!
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && !((outgoingUBX->cls != requestedClass) || (outgoingUBX->id != requestedID)))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: data being OVERWRITTEN after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_DATA_OVERWRITTEN); // Data was valid but has been or is being overwritten
      }

      // If packetAck.classAndIDmatch is VALID but both outgoingUBX->valid and outgoingUBX->classAndIDmatch
      // are NOT_VALID then we can be confident we have had a checksum failure on the data packet
      else if ((packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: CRC failed after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_CRC_FAIL); //Checksum fail
      }

      // If our packet was not-acknowledged (NACK) we do not receive a data packet - we only get the NACK.
      // So you would expect outgoingUBX->valid and outgoingUBX->classAndIDmatch to still be NOT_DEFINED
      // But if a full PVT packet arrives afterwards outgoingUBX->valid could be VALID (or just possibly NOT_VALID)
      // but outgoingUBX->cls and outgoingUBX->id would not match...
      // So I think this is telling us we need a special state for packetAck.classAndIDmatch to tell us
      // the packet was definitely NACK'd otherwise we are possibly just guessing...
      // Note: the addition of packetBuf changes the logic of this, but we'll leave the code as is for now.
      else if (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_NOTACKNOWLEDGED)
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: data was NOTACKNOWLEDGED (NACK) after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_COMMAND_NACK); //We received a NACK!
      }

      // If the outgoingUBX->classAndIDmatch is VALID but the packetAck.classAndIDmatch is NOT_VALID
      // then the ack probably had a checksum error. We will take a gamble and return DATA_RECEIVED.
      // If we were playing safe, we should return FAIL instead
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->cls == requestedClass) && (outgoingUBX->id == requestedID))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: VALID data and INVALID ACK received after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_DATA_RECEIVED); //We received valid data and an invalid ACK!
      }

      // If the outgoingUBX->classAndIDmatch is NOT_VALID and the packetAck.classAndIDmatch is NOT_VALID
      // then we return a FAIL. This must be a double checksum failure?
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: INVALID data and INVALID ACK received after ");
          printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec\r\n");
        }
        return (SFE_UBLOX_STATUS_FAIL); //We received invalid data and an invalid ACK!
      }

      // If the outgoingUBX->classAndIDmatch is VALID and the packetAck.classAndIDmatch is NOT_DEFINED
      // then the ACK has not yet been received and we should keep waiting for it
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED))
      {
        if (_printDebug == true)
        {
          printf("waitForACKResponse: valid data after ");
          printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec. Waiting for ACK.\r\n");
        }
      }

    } //checkUbloxInternal == true

    DelayMs(1);
  }

  // We have timed out...
  // If the outgoingUBX->classAndIDmatch is VALID then we can take a gamble and return DATA_RECEIVED
  // even though we did not get an ACK
  if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->cls == requestedClass) && (outgoingUBX->id == requestedID))
  {
    if (_printDebug == true)
    {
      printf("waitForACKResponse: TIMEOUT with valid data after ");
      printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
      printf(" msec. \r\n");
    }
    return (SFE_UBLOX_STATUS_DATA_RECEIVED); //We received valid data... But no ACK!
  }

  if (_printDebug == true)
  {
    printf("waitForACKResponse: TIMEOUT after ");
    printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
    printf(" msec. \r\n");
  }

  return (SFE_UBLOX_STATUS_TIMEOUT);
}

//For non-CFG queries no ACK is sent so we use this function
//Returns SFE_UBLOX_STATUS_DATA_RECEIVED if we got a config packet full of response data that has CLS/ID match to our query packet
//Returns SFE_UBLOX_STATUS_CRC_FAIL if we got a corrupt config packet that has CLS/ID match to our query packet
//Returns SFE_UBLOX_STATUS_TIMEOUT if we timed out
//Returns SFE_UBLOX_STATUS_DATA_OVERWRITTEN if we got an a valid packetCfg but that the packetCfg has been
// or is currently being overwritten (remember that Serial data can arrive very slowly)
sfe_ublox_status_e waitForNoACKResponse(ubxPacket *outgoingUBX, uint8_t requestedClass, uint8_t requestedID, uint16_t maxTime)
{
  outgoingUBX->valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED; //This will go VALID (or NOT_VALID) when we receive a response to the packet we sent
  packetAck.valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;
  packetBuf.valid = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;
  outgoingUBX->classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED; // This will go VALID (or NOT_VALID) when we receive a packet that matches the requested class and ID
  packetAck.classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;
  packetBuf.classAndIDmatch = SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED;

  unsigned long startTime = SysTimeToMs(SysTimeGet());
  while (SysTimeToMs(SysTimeGet()) - startTime < maxTime)
  {
    if (checkUbloxInternal(outgoingUBX, requestedClass, requestedID) == true) //See if new data is available. Process bytes as they come in.
    {

      // If outgoingUBX->classAndIDmatch is VALID
      // and outgoingUBX->valid is _still_ VALID and the class and ID _still_ match
      // then we can be confident that the data in outgoingUBX is valid
      if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->cls == requestedClass) && (outgoingUBX->id == requestedID))
      {
        if (_printDebug == true)
        {
          printf("waitForNoACKResponse: valid data with CLS/ID match after ");
          printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec\r\n");
        }
        return (SFE_UBLOX_STATUS_DATA_RECEIVED); //We received valid data!
      }

      // If the outgoingUBX->classAndIDmatch is VALID
      // but the outgoingUBX->cls or ID no longer match then we can be confident that we had
      // valid data but it has been or is currently being overwritten by another packet (e.g. PVT).
      // If (e.g.) a PVT packet is _being_ received: outgoingUBX->valid will be NOT_DEFINED
      // If (e.g.) a PVT packet _has been_ received: outgoingUBX->valid will be VALID (or just possibly NOT_VALID)
      // So we cannot use outgoingUBX->valid as part of this check.
      // Note: the addition of packetBuf should make this check redundant!
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && !((outgoingUBX->cls != requestedClass) || (outgoingUBX->id != requestedID)))
      {
        if (_printDebug == true)
        {
          printf("waitForNoACKResponse: data being OVERWRITTEN after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_DATA_OVERWRITTEN); // Data was valid but has been or is being overwritten
      }

      // If outgoingUBX->classAndIDmatch is NOT_DEFINED
      // and outgoingUBX->valid is VALID then this must be (e.g.) a PVT packet
      else if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID))
      {
        if (_printDebug == true)
        {
          printf("waitForNoACKResponse: valid but UNWANTED data after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec. Class: ");
          printf("%02X\r\n", outgoingUBX->cls);
          printf(" ID: ");
          printf("%02X\r\n", outgoingUBX->id);
        }
      }

      // If the outgoingUBX->classAndIDmatch is NOT_VALID then we return CRC failure
      else if (outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID)
      {
        if (_printDebug == true)
        {
          printf("waitForNoACKResponse: CLS/ID match but failed CRC after ");
          printf("%ld\r\n", SysTimeToMs(SysTimeGet()) - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_CRC_FAIL); //We received invalid data
      }
    }

    DelayMs(1);
  }

  if (_printDebug == true)
  {
    printf("waitForNoACKResponse: TIMEOUT after ");
    printf("%ld", SysTimeToMs(SysTimeGet()) - startTime);
    printf(" msec. No packet received.\r\n");
  }

  return (SFE_UBLOX_STATUS_TIMEOUT);
}

/**
 * @brief Specific commands are here.
 * 
 */

//Save current configuration to flash and BBR (battery backed RAM)
//This still works but it is the old way of configuring ublox modules. See getVal and setVal for the new methods
bool saveConfiguration(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_CFG;
  packetCfg.len = 12;
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint8_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  packetCfg.payload[4] = 0xFF; //Set any bit in the saveMask field to save current config to Flash and BBR
  packetCfg.payload[5] = 0xFF;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Save the selected configuration sub-sections to flash and BBR (battery backed RAM)
//This still works but it is the old way of configuring ublox modules. See getVal and setVal for the new methods
bool saveConfigSelective(uint32_t configMask, uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_CFG;
  packetCfg.len = 12;
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint8_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  packetCfg.payload[4] = configMask & 0xFF; //Set the appropriate bits in the saveMask field to save current config to Flash and BBR
  packetCfg.payload[5] = (configMask >> 8) & 0xFF;
  packetCfg.payload[6] = (configMask >> 16) & 0xFF;
  packetCfg.payload[7] = (configMask >> 24) & 0xFF;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Reset module to factory defaults
//This still works but it is the old way of configuring ublox modules. See getVal and setVal for the new methods
bool factoryDefault(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_CFG;
  packetCfg.len = 12;
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint8_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  packetCfg.payload[0] = 0xFF; //Set any bit in the clearMask field to clear saved config
  packetCfg.payload[1] = 0xFF;
  packetCfg.payload[8] = 0xFF; //Set any bit in the loadMask field to discard current config and rebuild from lower non-volatile memory layers
  packetCfg.payload[9] = 0xFF;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Loads the payloadCfg array with the current protocol bits located the UBX-CFG-PRT register for a given port
bool getPortSettings(uint8_t portID, uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_PRT;
  packetCfg.len = 1;
  packetCfg.startingSpot = 0;

  payloadCfg[0] = portID;

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_RECEIVED); // We are expecting data and an ACK
}

//Configure a given port to output UBX, NMEA, RTCM3 or a combination thereof
//Port 0=I2c, 1=UART1, 2=UART2, 3=USB, 4=SPI
//Bit:0 = UBX, :1=NMEA, :5=RTCM3
bool setPortOutput(uint8_t portID, uint8_t outStreamSettings, uint16_t maxWait)
{
  //Get the current config values for this port ID
  if (getPortSettings(portID, maxWait) == false)
    return (false); //Something went wrong. Bail.

  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_PRT;
  packetCfg.len = 20;
  packetCfg.startingSpot = 0;

  //payloadCfg is now loaded with current bytes. Change only the ones we need to
  payloadCfg[14] = outStreamSettings; //OutProtocolMask LSB - Set outStream bits

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Configure a given port to input UBX, NMEA, RTCM3 or a combination thereof
//Port 0=I2c, 1=UART1, 2=UART2, 3=USB, 4=SPI
//Bit:0 = UBX, :1=NMEA, :5=RTCM3
bool setPortInput(uint8_t portID, uint8_t inStreamSettings, uint16_t maxWait)
{
  //Get the current config values for this port ID
  //This will load the payloadCfg array with current port settings
  if (getPortSettings(portID, maxWait) == false)
    return (false); //Something went wrong. Bail.

  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_PRT;
  packetCfg.len = 20;
  packetCfg.startingSpot = 0;

  //payloadCfg is now loaded with current bytes. Change only the ones we need to
  payloadCfg[12] = inStreamSettings; //InProtocolMask LSB - Set inStream bits

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Configure a port to output UBX, NMEA, RTCM3 or a combination thereof
bool setI2COutput(uint8_t comSettings, uint16_t maxWait)
{
  return (setPortOutput(COM_PORT_I2C, comSettings, maxWait));
}
bool setUART1Output(uint8_t comSettings, uint16_t maxWait)
{
  return (setPortOutput(COM_PORT_UART1, comSettings, maxWait));
}

bool set_powersave_config(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_PM2;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //	,0x02 ,0x06 ,0x00 ,0x00 /* v2, reserved 1..3 */
  payloadCfg[packetCfg.len++] = 0x02; //
  payloadCfg[packetCfg.len++] = 0x06; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x60 ,0x90 ,0x40 ,0x01 /* ON/OFF tracking mode, update ephemeris */
  payloadCfg[packetCfg.len++] = 0x60; //
  payloadCfg[packetCfg.len++] = 0x90; //
  payloadCfg[packetCfg.len++] = 0x40; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //	,0x00 ,0x00 ,0x00 ,0x00 /* update period, 0 ms(infinity) */

  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x00 ,0x00 ,0x00 ,0x00 /* search period, 0 ms(infinity) */

  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x00 ,0x00 ,0x00 ,0x00 /* grid offset */

  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x02 ,0x00             /* on-time after first fix */
  payloadCfg[packetCfg.len++] = 0x02; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x00 ,0x00             /* minimum acquisition time */
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x2C ,0x01 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0x2C; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x4F ,0xC1 ,0x03 ,0x00

  payloadCfg[packetCfg.len++] = 0x4F; //
  payloadCfg[packetCfg.len++] = 0xC1; //
  payloadCfg[packetCfg.len++] = 0x03; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x87 ,0x02 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0x87; //
  payloadCfg[packetCfg.len++] = 0x02; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0xFF ,0x00 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0xFF; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x64 ,0x40 ,0x01 ,0x00

  payloadCfg[packetCfg.len++] = 0x64; //
  payloadCfg[packetCfg.len++] = 0x40; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x00; //

  //	,0x00 ,0x00 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //

  packetCfg.len = 0x30;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

// Put the GPS into On/OFF power save mode, with EXTINT pin to toggle power save mode and continueous mode
// Setting using UBX-CFG-RXM
// This is the original packet that was sent.
// static uint8_t set_power_save_mode[] =
// {0xB5 ,0x62 ,0x06 ,0x11 ,0x02 ,0x00 ,0x08 ,0x01 ,0x22 ,0x92};

bool put_in_power_save_mode(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RXM;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //0x08 ,0x01

  /* 1: Power Save Mode */
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x01; //

  packetCfg.len = 0x02;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

// Put the GPS into On/OFF power save mode, with EXTINT pin to toggle power save mode and continueous mode
// Setting using UBX-CFG-RXM
// This is the original packet that was sent.
// static uint8_t set_continueous_mode[] =
// {0xB5 ,0x62 ,0x06 ,0x11 ,0x02 ,0x00 ,0x08 ,0x00 ,0x21 ,0x91};

bool put_in_continueous_mode(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RXM;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //0x08 ,0x00

  /* 1: Power Save Mode */
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x00; //

  packetCfg.len = 0x02;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

// Use only the American GPS constellation. Setting using UBX-CFG-GNSS
// This is the original packet that was sent.
//static uint8_t setGPSonly[] = {
//		0xB5,0x62,0x06,0x3E,0x3C,0x00,              /* UBX-CFG-GNSS */
//		0x00,0x00,0x20,0x07,                        /* use 32 channels, 7 configs following */
//		0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,    /* GPS enable */
//		0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,	/* SBAS disable */
//		0x02,0x04,0x08,0x00,0x00,0x00,0x01,0x01,	/* Galileo disable */
//		0x03,0x08,0x10,0x00,0x00,0x00,0x01,0x01,	/* Beidou disable */
//		0x04,0x00,0x08,0x00,0x00,0x00,0x01,0x01,	/* IMES disable */
//		0x05,0x00,0x03,0x00,0x01,0x00,0x01,0x01,	/* QZSS enable */
//		0x06,0x08,0x0E,0x00,0x00,0x00,0x01,0x01,	/* GLONASS disable */
//		0x2D,0x59                                   /* checksum */
//};

bool setGPS_constellation_only(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_GNSS;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //		0x00,0x00,0x20,0x07,                        /* use 32 channels, 7 configs following */

  /* use 32 channels, 7 configs following */
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x20; //
  payloadCfg[packetCfg.len++] = 0x07; //

  //		0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,    /* GPS enable */

  /* GPS enable */
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x10; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //		0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,	/* SBAS disable */

  /* SBAS disable */
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x03; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //		0x02,0x04,0x08,0x00,0x00,0x00,0x01,0x01,	/* Galileo disable */
  /* Galileo disable */
  payloadCfg[packetCfg.len++] = 0x02; //
  payloadCfg[packetCfg.len++] = 0x04; //
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //		0x03,0x08,0x10,0x00,0x00,0x00,0x01,0x01,	/* Beidou disable */
  /* Beidou disable */
  payloadCfg[packetCfg.len++] = 0x03; //
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x10; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //		0x04,0x00,0x08,0x00,0x00,0x00,0x01,0x01,	/* IMES disable */
  /* IMES disable */
  payloadCfg[packetCfg.len++] = 0x04; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //		0x05,0x00,0x03,0x00,0x01,0x00,0x01,0x01,	/* QZSS enable */
  /* QZSS enable */
  payloadCfg[packetCfg.len++] = 0x05; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x03; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  //		0x06,0x08,0x0E,0x00,0x00,0x00,0x01,0x01,	/* GLONASS disable */

  /* GLONASS disable */
  payloadCfg[packetCfg.len++] = 0x06; //
  payloadCfg[packetCfg.len++] = 0x08; //
  payloadCfg[packetCfg.len++] = 0x0E; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x00; //
  payloadCfg[packetCfg.len++] = 0x01; //
  payloadCfg[packetCfg.len++] = 0x01; //

  packetCfg.len = 0x3C;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Change the dynamic platform model using UBX-CFG-NAV5
//Possible values are:
//PORTABLE,STATIONARY,PEDESTRIAN,AUTOMOTIVE,SEA,
//AIRBORNE1g,AIRBORNE2g,AIRBORNE4g,WRIST,BIKE
//WRIST is not supported in protocol versions less than 18
//BIKE is supported in protocol versions 19.2
bool setDynamicModel(dynModel newDynamicModel, uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_NAV5;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //Ask module for the current navigation model settings. Loads into payloadCfg.
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (false);

  payloadCfg[0] = 0x01;            // mask: set only the dyn bit (0)
  payloadCfg[1] = 0x00;            // mask
  payloadCfg[2] = newDynamicModel; // dynModel

  packetCfg.len = 36;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Get the dynamic platform model using UBX-CFG-NAV5
//Returns 255 if the sendCommand fails
uint8_t getDynamicModel(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_NAV5;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //Ask module for the current navigation model settings. Loads into payloadCfg.
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (255);

  return (payloadCfg[2]); // Return the dynamic model
}

//Given a spot in the payload array, extract four bytes and build a long
uint32_t extractLong(uint8_t spotToStart)
{
  uint32_t val = 0;
  val |= (uint32_t)payloadCfg[spotToStart + 0] << 8 * 0;
  val |= (uint32_t)payloadCfg[spotToStart + 1] << 8 * 1;
  val |= (uint32_t)payloadCfg[spotToStart + 2] << 8 * 2;
  val |= (uint32_t)payloadCfg[spotToStart + 3] << 8 * 3;
  return (val);
}

//Given a spot in the payload array, extract two bytes and build an int
uint16_t extractInt(uint8_t spotToStart)
{
  uint16_t val = 0;
  val |= (uint16_t)payloadCfg[spotToStart + 0] << 8 * 0;
  val |= (uint16_t)payloadCfg[spotToStart + 1] << 8 * 1;
  return (val);
}

//Given a spot, extract a byte from the payload
uint8_t extractByte(uint8_t spotToStart)
{
  return (payloadCfg[spotToStart]);
}

//Given a spot, extract a signed 8-bit value from the payload
int8_t extractSignedChar(uint8_t spotToStart)
{
  return ((int8_t)payloadCfg[spotToStart]);
}

//Get the current year
uint16_t getYear(uint16_t maxWait)
{
  if (moduleQueried.gpsYear == false)
    getPVT(maxWait);
  moduleQueried.gpsYear = false; //Since we are about to give this to user, mark this data as stale
  return (gpsYear);
}

//Get the current month
uint8_t getMonth(uint16_t maxWait)
{
  if (moduleQueried.gpsMonth == false)
    getPVT(maxWait);
  moduleQueried.gpsMonth = false; //Since we are about to give this to user, mark this data as stale
  return (gpsMonth);
}

//Get the current day
uint8_t getDay(uint16_t maxWait)
{
  if (moduleQueried.gpsDay == false)
    getPVT(maxWait);
  moduleQueried.gpsDay = false; //Since we are about to give this to user, mark this data as stale
  return (gpsDay);
}

//Get the current hour
uint8_t getHour(uint16_t maxWait)
{
  if (moduleQueried.gpsHour == false)
    getPVT(maxWait);
  moduleQueried.gpsHour = false; //Since we are about to give this to user, mark this data as stale
  return (gpsHour);
}

//Get the current minute
uint8_t getMinute(uint16_t maxWait)
{
  if (moduleQueried.gpsMinute == false)
    getPVT(maxWait);
  moduleQueried.gpsMinute = false; //Since we are about to give this to user, mark this data as stale
  return (gpsMinute);
}

//Get the current second
uint8_t getSecond(uint16_t maxWait)
{
  if (moduleQueried.gpsSecond == false)
    getPVT(maxWait);
  moduleQueried.gpsSecond = false; //Since we are about to give this to user, mark this data as stale
  return (gpsSecond);
}

//Get the current millisecond
uint16_t getMillisecond(uint16_t maxWait)
{
  if (moduleQueried.gpsiTOW == false)
    getPVT(maxWait);
  moduleQueried.gpsiTOW = false; //Since we are about to give this to user, mark this data as stale
  return (gpsMillisecond);
}

//Get the current nanoseconds - includes milliseconds
int32_t getNanosecond(uint16_t maxWait)
{
  if (moduleQueried.gpsNanosecond == false)
    getPVT(maxWait);
  moduleQueried.gpsNanosecond = false; //Since we are about to give this to user, mark this data as stale
  return (gpsNanosecond);
}

//Get the latest Position/Velocity/Time solution and fill all global variables
bool getPVT(uint16_t maxWait)
{
  if (autoPVT && autoPVTImplicitUpdate)
  {
    //The GPS is automatically reporting, we just check whether we got unread data
    if (_printDebug == true)
    {
      printf("getPVT: Autoreporting");
    }
    checkUbloxInternal(&packetCfg, UBX_CLASS_NAV, UBX_NAV_PVT);
    return moduleQueried.all;
  }
  else if (autoPVT && !autoPVTImplicitUpdate)
  {
    //Someone else has to call checkUblox for us...
    if (_printDebug == true)
    {
      printf("getPVT: Exit immediately");
    }
    return (false);
  }
  else
  {
    if (_printDebug == true)
    {
      printf("getPVT: Polling");
    }

    //The GPS is not automatically reporting navigation position so we have to poll explicitly
    packetCfg.cls = UBX_CLASS_NAV;
    packetCfg.id = UBX_NAV_PVT;
    packetCfg.len = 0;
    //packetCfg.startingSpot = 20; //Begin listening at spot 20 so we can record up to 20+MAX_PAYLOAD_SIZE = 84 bytes Note:now hard-coded in processUBX

    //The data is parsed as part of processing the response
    sfe_ublox_status_e retVal = sendCommand(&packetCfg, maxWait);

    if (retVal == SFE_UBLOX_STATUS_DATA_RECEIVED)
      return (true);

    if (_printDebug == true)
    {
      printf("getPVT retVal: ");
      printf("%s", statusString(retVal));
    }
    return (false);
  }
}

uint32_t getTimeOfWeek(uint16_t maxWait /* = 250*/)
{
  if (moduleQueried.gpsiTOW == false)
    getPVT(maxWait);
  moduleQueried.gpsiTOW = false; //Since we are about to give this to user, mark this data as stale
  return (timeOfWeek);
}

//Get the current 3D high precision positional accuracy - a fun thing to watch
//Returns a long representing the 3D accuracy in millimeters
uint32_t getPositionAccuracy(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_NAV;
  packetCfg.id = UBX_NAV_HPPOSECEF;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are only expecting data (no ACK)
    return (0);                                                           //If command send fails then bail

  uint32_t tempAccuracy = extractLong(24); //We got a response, now extract a long beginning at a given position

  if ((tempAccuracy % 10) >= 5)
    tempAccuracy += 5; //Round fraction of mm up to next mm if .5 or above
  tempAccuracy /= 10;  //Convert 0.1mm units to mm

  return (tempAccuracy);
}

//Get the current date validity
bool getDateValid(uint16_t maxWait)
{
  if (moduleQueried.gpsDateValid == false)
    getPVT(maxWait);
  moduleQueried.gpsDateValid = false; //Since we are about to give this to user, mark this data as stale
  return (gpsDateValid);
}

//Get the current time validity
bool getTimeValid(uint16_t maxWait)
{
  if (moduleQueried.gpsTimeValid == false)
    getPVT(maxWait);
  moduleQueried.gpsTimeValid = false; //Since we are about to give this to user, mark this data as stale
  return (gpsTimeValid);
}

//Get the current latitude in degrees
//Returns a long representing the number of degrees *10^-7
int32_t getLatitude(uint16_t maxWait)
{
  if (moduleQueried.latitude == false)
    getPVT(maxWait);
  moduleQueried.latitude = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (latitude);
}

//Get the current longitude in degrees
//Returns a long representing the number of degrees *10^-7
int32_t getLongitude(uint16_t maxWait)
{
  if (moduleQueried.longitude == false)
    getPVT(maxWait);
  moduleQueried.longitude = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (longitude);
}

//Get the current altitude in mm according to ellipsoid model
int32_t getAltitude(uint16_t maxWait)
{
  if (moduleQueried.altitude == false)
    getPVT(maxWait);
  moduleQueried.altitude = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (altitude);
}

//Get the current altitude in mm according to mean sea level
//Ellipsoid model: https://www.esri.com/news/arcuser/0703/geoid1of3.html
//Difference between Ellipsoid Model and Mean Sea Level: https://eos-gnss.com/elevation-for-beginners/
int32_t getAltitudeMSL(uint16_t maxWait)
{
  if (moduleQueried.altitudeMSL == false)
    getPVT(maxWait);
  moduleQueried.altitudeMSL = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (altitudeMSL);
}

//Get the number of satellites used in fix
uint8_t getSIV(uint16_t maxWait)
{
  if (moduleQueried.SIV == false)
    getPVT(maxWait);
  moduleQueried.SIV = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (SIV);
}

//Get the current fix type
//0=no fix, 1=dead reckoning, 2=2D, 3=3D, 4=GNSS, 5=Time fix
uint8_t getFixType(uint16_t maxWait)
{
  if (moduleQueried.fixType == false)
  {
    getPVT(maxWait);
  }
  moduleQueried.fixType = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (fixType);
}

//Get the current gnssFixOK status
//0= not valid fix, 1 = not valid fix
uint8_t getgnssFixOK(uint16_t maxWait)
{
  if (moduleQueried.gnssFixOK == false)
  {
    getPVT(maxWait);
  }
  moduleQueried.gnssFixOK = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (gnssFixOK);
}

//Mark all the PVT data as read/stale. This is handy to get data alignment after CRC failure
void flushPVT()
{
  //Mark all datums as stale (read before)
  moduleQueried.gpsiTOW = false;
  moduleQueried.gpsYear = false;
  moduleQueried.gpsMonth = false;
  moduleQueried.gpsDay = false;
  moduleQueried.gpsHour = false;
  moduleQueried.gpsMinute = false;
  moduleQueried.gpsSecond = false;
  moduleQueried.gpsDateValid = false;
  moduleQueried.gpsTimeValid = false;
  moduleQueried.gpsNanosecond = false;

  moduleQueried.all = false;
  moduleQueried.longitude = false;
  moduleQueried.latitude = false;
  moduleQueried.altitude = false;
  moduleQueried.altitudeMSL = false;
  moduleQueried.SIV = false;
  moduleQueried.fixType = false;
  moduleQueried.gnssFixOK = false;
  moduleQueried.carrierSolution = false;
  moduleQueried.groundSpeed = false;
  moduleQueried.headingOfMotion = false;
  moduleQueried.pDOP = false;
}

void factoryReset()
{
  // Copy default settings to permanent
  // Note: this does not load the permanent configuration into the current configuration. Calling factoryDefault() will do that.
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_CFG;
  packetCfg.len = 13;
  packetCfg.startingSpot = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    payloadCfg[0 + i] = 0xff; // clear mask: copy default config to permanent config
    payloadCfg[4 + i] = 0x00; // save mask: don't save current to permanent
    payloadCfg[8 + i] = 0x00; // load mask: don't copy permanent config to current
  }
  payloadCfg[12] = 0xff;      // all forms of permanent memory
  sendCommand(&packetCfg, 0); // don't expect ACK
  hardReset();                // cause factory default config to actually be loaded and used cleanly
}

void hardReset()
{
  // Issue hard reset
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RST;
  packetCfg.len = 4;
  packetCfg.startingSpot = 0;
  payloadCfg[0] = 0xff;       // cold start
  payloadCfg[1] = 0xff;       // cold start
  payloadCfg[2] = 0;          // 0=HW reset
  payloadCfg[3] = 0;          // reserved
  sendCommand(&packetCfg, 0); // don't expect ACK
}

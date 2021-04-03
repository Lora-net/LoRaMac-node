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
#if 1


#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "i2c.h"
#include "SparkFun_Ublox_Arduino_Library.h"

extern I2c_t I2c;





static uint8_t GPS_buffer[MAX_PAYLOAD_SIZE];
uint8_t ubx_packet_buff[MAX_PAYLOAD_SIZE] = {0};



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


// original uint8_t resetReceiver[12]		= {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xB9, 0x00, 0x00, 0xC6, 0x8B}; 
void ihardReset()
{
  // Issue hard reset
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RST;
  packetCfg.len = 4;
  packetCfg.startingSpot = 0;
  payloadCfg[0] = 0xff;       // cold start
  payloadCfg[1] = 0xb9;       // cold start
  payloadCfg[2] = 0;          // 0=HW reset
  payloadCfg[3] = 0;          // reserved
  sendCommand(&packetCfg, 0); // don't expect ACK
}


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
  if (HAL_GetTick() - lastCheck >= i2cPollingWait)
  {
    //Get the number of bytes available from the module
    uint16_t bytesAvailable = 0;
//    _i2cPort->beginTransmission(_gpsI2Caddress);
//    _i2cPort->write(0xFD);                     //0xFD (MSB) and 0xFE (LSB) are the registers that contain number of bytes available
//    if (_i2cPort->endTransmission(false) != 0) //Send a restart command. Do not release bus.
//      return (false);                          //Sensor did not ACK

		
		// if (HAL_I2C_IsDeviceReady(&hi2c1,(uint16_t) _gpsI2Caddress << 1,5,defaultMaxWait) != HAL_OK)
		// {
		// 	return (false);                          //Sensor did not ACK
		// }
		
		uint8_t buff_rx[2] = {0};
		
		//uint16_t return_value = 0;
		if (I2cReadMemBuffer(&I2c,(uint16_t) _gpsI2Caddress << 1,(uint16_t)0xFD,buff_rx,2 ) != LMN_STATUS_OK)
		{
			return (false);                          //Sensor did not ACK
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

			lastCheck = HAL_GetTick(); //Put off checking to avoid I2C bus traffic
			return (false);
		}
		bytesAvailable = (uint16_t)msb << 8 | lsb;
    

    if (bytesAvailable == 0)
    {
      if (_printDebug == true)
      {
        printf("checkUbloxI2C: OK, zero bytes available\r\n");
      }
      lastCheck = HAL_GetTick(); //Put off checking to avoid I2C bus traffic
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
        printf("%d\r\n",bytesAvailable);
      }
    }

    if (bytesAvailable > 100)
    {
      if (_printDebug == true)
      {
        printf("checkUbloxI2C: Large packet of ");
        printf("%d",bytesAvailable);
        printf(" bytes received\r\n");
      }
    }
    else
    {
      if (_printDebug == true)
      {
        printf("checkUbloxI2C: Reading ");
        printf("%d",bytesAvailable);
        printf(" bytes\r\n");
      }
    }
		


		if (I2cReadMemBuffer(&I2c,(uint16_t) _gpsI2Caddress << 1,(uint16_t)0xFF,ubx_packet_buff,bytesAvailable ) != LMN_STATUS_OK)
		{
			return (false);  //Sensor did not ACK
		}
		for(int i = 0; i < bytesAvailable ; i++)
		{
			uint8_t incoming = ubx_packet_buff[i];
			process(incoming, incomingUBX, requestedClass, requestedID); //Process this valid character
		}
  }

  return (true);

} //end checkUbloxI2C()


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
          printf("%02X",packetBuf.cls);
          printf(" ID: 0x");
          printf("%02X\r\n",packetBuf.id);
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
            printf("%02X ",packetBuf.payload[0]);
            printf(" ID: 0x");
            printf("%02X ",packetBuf.payload[1]);
            printf(" len: ");
            printf("%02X\r\n",packetBuf.len);
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
    processNMEA(incoming); //Process each NMEA character
  }
  else if (currentSentence == RTCM)
  {
    processRTCMframe(incoming); //Deal with RTCM bytes
  }
}

//This is the default or generic NMEA processor. We're only going to pipe the data to serial port so we can see it.
//User could overwrite this function to pipe characters to nmea.process(c) of tinyGPS or MicroNMEA
//Or user could pipe each character to a buffer, radio, etc.
void processNMEA(char incoming)
{
//  //If user has assigned an output port then pipe the characters there
//  if (_nmeaOutputPort != NULL)
    printf("%c",incoming); //Echo this byte to the serial port
}

//We need to be able to identify an RTCM packet and then the length
//so that we know when the RTCM message is completely received and we then start
//listening for other sentences (like NMEA or UBX)
//RTCM packet structure is very odd. I never found RTCM STANDARD 10403.2 but
//http://d1.amobbs.com/bbs_upload782111/files_39/ourdev_635123CK0HJT.pdf is good
//https://dspace.cvut.cz/bitstream/handle/10467/65205/F3-BP-2016-Shkalikava-Anastasiya-Prenos%20polohove%20informace%20prostrednictvim%20datove%20site.pdf?sequence=-1
//Lead me to: https://forum.u-blox.com/index.php/4348/how-to-read-rtcm-messages-from-neo-m8p
//RTCM 3.2 bytes look like this:
//Byte 0: Always 0xD3
//Byte 1: 6-bits of zero
//Byte 2: 10-bits of length of this packet including the first two-ish header bytes, + 6.
//byte 3 + 4 bits: Msg type 12 bits
//Example: D3 00 7C 43 F0 ... / 0x7C = 124+6 = 130 bytes in this packet, 0x43F = Msg type 1087
void processRTCMframe(uint8_t incoming)
{
  if (rtcmFrameCounter == 1)
  {
    rtcmLen = (incoming & 0x03) << 8; //Get the last two bits of this byte. Bits 8&9 of 10-bit length
  }
  else if (rtcmFrameCounter == 2)
  {
    rtcmLen |= incoming; //Bits 0-7 of packet length
    rtcmLen += 6;        //There are 6 additional bytes of what we presume is header, msgType, CRC, and stuff
  }
  /*else if (rtcmFrameCounter == 3)
  {
    rtcmMsgType = incoming << 4; //Message Type, MS 4 bits
  }
  else if (rtcmFrameCounter == 4)
  {
    rtcmMsgType |= (incoming >> 4); //Message Type, bits 0-7
  }*/

  rtcmFrameCounter++;

  processRTCM(incoming); //Here is where we expose this byte to the user

  if (rtcmFrameCounter == rtcmLen)
  {
    //We're done!
    currentSentence = NONE; //Reset and start looking for next sentence type
  }
}

//This function is called for each byte of an RTCM frame
//Ths user can overwrite this function and process the RTCM frame as they please
//Bytes can be piped to Serial or other interface. The consumer could be a radio or the internet (Ntrip broadcaster)
void processRTCM(uint8_t incoming)
{
  //Radio.sendReliable((String)incoming); //An example of passing this byte to a radio

  //write(incoming); //An example of passing this byte out the serial port

  //Debug printing
  //  printf(" "));
  //  iincoming < 0x10) printf("0"));
  //  iincoming < 0x10) printf("0"));
  //  printf(incoming, HEX);
  //  irtcmFrameCounter % 16 == 0) printf();
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
          printf("%02X ",incomingUBX->payload[0]);
          printf(" Requested ID: 0x");
          printf("%02X \r\n",incomingUBX->payload[1]);
        }
      }

      if (_printDebug == true)
      {
        printf("Incoming: Size: ");
        printf("%d",incomingUBX->len);
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
        printf("%d",incomingUBX->checksumA);
        printf(" checksumB: ");
        printf("%d",incomingUBX->checksumB);

        printf(" rollingChecksumA: ");
        printf("%d",rollingChecksumA);
        printf(" rollingChecksumB: ");
        printf("%d",rollingChecksumB);
        printf("\r\n");

        printf("Failed  : ");
        printf("Size: ");
        printf("%d",incomingUBX->len);
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
      carrierSolution = extractByte(21 - startingSpot) >> 6; //Get 6th&7th bits of this byte
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
        printf("%3.5f\n",((float)extractLong(4)) / 1000.0f);
        printf(" ");
        printf("LON: ");
        printf("%3.5f\n",((float)(int32_t)extractLong(8)) / 10000000.0f);
        printf(" ");
        printf("LAT: ");
        printf("%3.5f\n",((float)(int32_t)extractLong(12)) / 10000000.0f);
        printf(" ");
        printf("ELI M: ");
        printf("%3.5f\n",((float)(int32_t)extractLong(16)) / 1000.0f);
        printf(" ");
        printf("MSL M: ");
        printf("%3.5f\n",((float)(int32_t)extractLong(20)) / 1000.0f);
        printf(" ");
        printf("LON HP: ");
        printf("%d\r\n",extractSignedChar(24));
        printf(" ");
        printf("LAT HP: ");
        printf("%d\r\n",extractSignedChar(25));
        printf(" ");
        printf("ELI HP: ");
        printf("%d\r\n",extractSignedChar(26));
        printf(" ");
        printf("MSL HP: ");
        printf("%d\r\n",extractSignedChar(27));
        printf(" ");
        printf("HA 2D M: ");
        printf("%3.5f\n",((float)(int32_t)extractLong(28)) / 10000.0f);
        printf(" ");
        printf("VERT M: ");
        printf("%3.5f\n",((float)(int32_t)extractLong(32)) / 10000.0f);
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
	GPS_buffer[cnt++] = UBX_SYNCH_1;                            //μ - oh ublox, you're funny. I will call you micro-blox from now on.
	GPS_buffer[cnt++] = UBX_SYNCH_2;                            //b
	GPS_buffer[cnt++] = outgoingUBX->cls;
	GPS_buffer[cnt++] = outgoingUBX->id;
	GPS_buffer[cnt++] = outgoingUBX->len & 0xFF;   //LSB
	GPS_buffer[cnt++] = outgoingUBX->len >> 8;     //MSB

	for (uint16_t x = 0; x < outgoingUBX->len; x++)
	{
		GPS_buffer[cnt++] = outgoingUBX->payload[x];
	}
	
	GPS_buffer[cnt++] = outgoingUBX->checksumA;
	GPS_buffer[cnt++] = outgoingUBX->checksumB;
	
	
	return ((I2cWriteMemBuffer(&I2c, _gpsI2Caddress << 1,(uint16_t)0xFF, GPS_buffer, cnt) == LMN_STATUS_OK)
		       ? SFE_UBLOX_STATUS_SUCCESS : SFE_UBLOX_STATUS_I2C_COMM_FAILURE);
}


//Returns true if I2C device ack's
bool isConnected(uint16_t maxWait)
{

  // Query navigation rate to see whether we get a meaningful response
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RATE;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_RECEIVED); // We are polling the RATE so we expect data and an ACK
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
      printf("%d",packet->cls);
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
      printf("%02X",packet->id);
    }

    printf(" Len: 0x");
    printf("%02X",packet->len);

    // Only printf the payload is ignoreThisPayload is false otherwise
    // we could be printing gibberish from beyond the end of packetBuf
    if (ignoreThisPayload == false)
    {
      printf(" Payload:");

      for (int x = 0; x < packet->len; x++)
      {
        printf("0x");
        printf("%02X ",packet->payload[x]);
      }
    }
    else
    {
      printf(" Payload: IGNORED");
    }
    printf("\r\n");
  }
}

//=-=-=-=-=-=-=-= Specific commands =-=-=-=-=-=-=-==-=-=-=-=-=-=-=
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//When messages from the class CFG are sent to the receiver, the receiver will send an "acknowledge"(UBX - ACK - ACK) or a
//"not acknowledge"(UBX-ACK-NAK) message back to the sender, depending on whether or not the message was processed correctly.
//Some messages from other classes also use the same acknowledgement mechanism.

//When we poll or get a setting, we will receive _both_ a config packet and an ACK
//If the poll or get request is not valid, we will receive _only_ a NACK

//If we are trying to get or poll a setting, then packetCfg.len will be 0 or 1 when the packetCfg is _sent_.
//If we poll the setting for a particular port using UBX-CFG-PRT then .len will be 1 initially
//For all other gets or polls, .len will be 0 initially
//(It would be possible for .len to be 2 _if_ we were using UBX-CFG-MSG to poll the settings for a particular message - but we don't use that (currently))

//If the get or poll _fails_, i.e. is NACK'd, then packetCfg.len could still be 0 or 1 after the NACK is received
//But if the get or poll is ACK'd, then packetCfg.len will have been updated by the incoming data and will always be at least 2

//If we are going to set the value for a setting, then packetCfg.len will be at least 3 when the packetCfg is _sent_.
//(UBX-CFG-MSG appears to have the shortest set length of 3 bytes)

//We need to think carefully about how interleaved PVT packets affect things.
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

  unsigned long startTime = HAL_GetTick();
  while (HAL_GetTick() - startTime < maxTime)
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
          printf("%d",HAL_GetTick() - startTime);
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
          printf("%d",HAL_GetTick() - startTime);
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
          printf("%d\r\n",HAL_GetTick() - startTime);
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
          printf("%d\r\n",HAL_GetTick() - startTime);
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
          printf("%d\r\n",HAL_GetTick() - startTime);
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
          printf("%d\r\n",HAL_GetTick() - startTime);
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
          printf("%d",HAL_GetTick() - startTime);
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
          printf("%d",HAL_GetTick() - startTime);
          printf(" msec. Waiting for ACK.\r\n");
        }
      }

    } //checkUbloxInternal == true
		
		HAL_Delay(1);
    //delayMicroseconds(500);
  }

  // We have timed out...
  // If the outgoingUBX->classAndIDmatch is VALID then we can take a gamble and return DATA_RECEIVED
  // even though we did not get an ACK
  if ((outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_VALID) && (packetAck.classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED) && (outgoingUBX->valid == SFE_UBLOX_PACKET_VALIDITY_VALID) && (outgoingUBX->cls == requestedClass) && (outgoingUBX->id == requestedID))
  {
    if (_printDebug == true)
    {
      printf("waitForACKResponse: TIMEOUT with valid data after ");
      printf("%d",HAL_GetTick() - startTime);
      printf(" msec. \r\n");
    }
    return (SFE_UBLOX_STATUS_DATA_RECEIVED); //We received valid data... But no ACK!
  }

  if (_printDebug == true)
  {
    printf("waitForACKResponse: TIMEOUT after ");
    printf("%d",HAL_GetTick() - startTime);
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

  unsigned long startTime = HAL_GetTick();
  while (HAL_GetTick() - startTime < maxTime)
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
          printf("%d",HAL_GetTick() - startTime);
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
          printf("%d\r\n",HAL_GetTick() - startTime);
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
          printf("%d\r\n",HAL_GetTick() - startTime);
          printf(" msec. Class: ");
          printf("%02X\r\n",outgoingUBX->cls);
          printf(" ID: ");
          printf("%02X\r\n",outgoingUBX->id);
        }
      }

      // If the outgoingUBX->classAndIDmatch is NOT_VALID then we return CRC failure
      else if (outgoingUBX->classAndIDmatch == SFE_UBLOX_PACKET_VALIDITY_NOT_VALID)
      {
        if (_printDebug == true)
        {
          printf("waitForNoACKResponse: CLS/ID match but failed CRC after ");
          printf("%d\r\n",HAL_GetTick() - startTime);
          printf(" msec");
        }
        return (SFE_UBLOX_STATUS_CRC_FAIL); //We received invalid data
      }
    }

    HAL_Delay(1);
  }

  if (_printDebug == true)
  {
    printf("waitForNoACKResponse: TIMEOUT after ");
    printf("%d",HAL_GetTick() - startTime);
    printf(" msec. No packet received.\r\n");
  }

  return (SFE_UBLOX_STATUS_TIMEOUT);
}

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


//Given a key, set a 16-bit value
//This function takes a full 32-bit key
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t setVal(uint32_t key, uint16_t value, uint8_t layer, uint16_t maxWait)
{
  return setVal16(key, value, layer, maxWait);
}

//Given a key, set a 16-bit value
//This function takes a full 32-bit key
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t setVal16(uint32_t key, uint16_t value, uint8_t layer, uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_VALSET;
  packetCfg.len = 4 + 4 + 2; //4 byte header, 4 byte key ID, 2 bytes of value
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint16_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  payloadCfg[0] = 0;     //Message Version - set to 0
  payloadCfg[1] = layer; //By default we ask for the BBR layer

  //Load key into outgoing payload
  payloadCfg[4] = key >> 8 * 0; //Key LSB
  payloadCfg[5] = key >> 8 * 1;
  payloadCfg[6] = key >> 8 * 2;
  payloadCfg[7] = key >> 8 * 3;

  //Load user's value
  payloadCfg[8] = value >> 8 * 0; //Value LSB
  payloadCfg[9] = value >> 8 * 1;

  //Send VALSET command with this key and value
  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Given a key, set an 8-bit value
//This function takes a full 32-bit key
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t setVal8(uint32_t key, uint8_t value, uint8_t layer, uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_VALSET;
  packetCfg.len = 4 + 4 + 1; //4 byte header, 4 byte key ID, 1 byte value
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint16_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  payloadCfg[0] = 0;     //Message Version - set to 0
  payloadCfg[1] = layer; //By default we ask for the BBR layer

  //Load key into outgoing payload
  payloadCfg[4] = key >> 8 * 0; //Key LSB
  payloadCfg[5] = key >> 8 * 1;
  payloadCfg[6] = key >> 8 * 2;
  payloadCfg[7] = key >> 8 * 3;

  //Load user's value
  payloadCfg[8] = value; //Value

  //Send VALSET command with this key and value
  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Given a key, set a 32-bit value
//This function takes a full 32-bit key
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t setVal32(uint32_t key, uint32_t value, uint8_t layer, uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_VALSET;
  packetCfg.len = 4 + 4 + 4; //4 byte header, 4 byte key ID, 4 bytes of value
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint16_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  payloadCfg[0] = 0;     //Message Version - set to 0
  payloadCfg[1] = layer; //By default we ask for the BBR layer

  //Load key into outgoing payload
  payloadCfg[4] = key >> 8 * 0; //Key LSB
  payloadCfg[5] = key >> 8 * 1;
  payloadCfg[6] = key >> 8 * 2;
  payloadCfg[7] = key >> 8 * 3;

  //Load user's value
  payloadCfg[8] = value >> 8 * 0; //Value LSB
  payloadCfg[9] = value >> 8 * 1;
  payloadCfg[10] = value >> 8 * 2;
  payloadCfg[11] = value >> 8 * 3;

  //Send VALSET command with this key and value
  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Start defining a new UBX-CFG-VALSET ubxPacket
//This function takes a full 32-bit key and 32-bit value
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t newCfgValset32(uint32_t key, uint32_t value, uint8_t layer)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_VALSET;
  packetCfg.len = 4 + 4 + 4; //4 byte header, 4 byte key ID, 4 bytes of value
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint16_t x = 0; x < MAX_PAYLOAD_SIZE; x++)
    packetCfg.payload[x] = 0;

  payloadCfg[0] = 0;     //Message Version - set to 0
  payloadCfg[1] = layer; //By default we ask for the BBR layer

  //Load key into outgoing payload
  payloadCfg[4] = key >> 8 * 0; //Key LSB
  payloadCfg[5] = key >> 8 * 1;
  payloadCfg[6] = key >> 8 * 2;
  payloadCfg[7] = key >> 8 * 3;

  //Load user's value
  payloadCfg[8] = value >> 8 * 0; //Value LSB
  payloadCfg[9] = value >> 8 * 1;
  payloadCfg[10] = value >> 8 * 2;
  payloadCfg[11] = value >> 8 * 3;

  //All done
  return (true);
}

//Start defining a new UBX-CFG-VALSET ubxPacket
//This function takes a full 32-bit key and 16-bit value
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t newCfgValset16(uint32_t key, uint16_t value, uint8_t layer)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_VALSET;
  packetCfg.len = 4 + 4 + 2; //4 byte header, 4 byte key ID, 2 bytes of value
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint16_t x = 0; x < MAX_PAYLOAD_SIZE; x++)
    packetCfg.payload[x] = 0;

  payloadCfg[0] = 0;     //Message Version - set to 0
  payloadCfg[1] = layer; //By default we ask for the BBR layer

  //Load key into outgoing payload
  payloadCfg[4] = key >> 8 * 0; //Key LSB
  payloadCfg[5] = key >> 8 * 1;
  payloadCfg[6] = key >> 8 * 2;
  payloadCfg[7] = key >> 8 * 3;

  //Load user's value
  payloadCfg[8] = value >> 8 * 0; //Value LSB
  payloadCfg[9] = value >> 8 * 1;

  //All done
  return (true);
}

//Start defining a new UBX-CFG-VALSET ubxPacket
//This function takes a full 32-bit key and 8-bit value
//Default layer is BBR
//Configuration of modern Ublox modules is now done via getVal/setVal/delVal, ie protocol v27 and above found on ZED-F9P
uint8_t newCfgValset8(uint32_t key, uint8_t value, uint8_t layer)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_VALSET;
  packetCfg.len = 4 + 4 + 1; //4 byte header, 4 byte key ID, 1 byte value
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint16_t x = 0; x < MAX_PAYLOAD_SIZE; x++)
    packetCfg.payload[x] = 0;

  payloadCfg[0] = 0;     //Message Version - set to 0
  payloadCfg[1] = layer; //By default we ask for the BBR layer

  //Load key into outgoing payload
  payloadCfg[4] = key >> 8 * 0; //Key LSB
  payloadCfg[5] = key >> 8 * 1;
  payloadCfg[6] = key >> 8 * 2;
  payloadCfg[7] = key >> 8 * 3;

  //Load user's value
  payloadCfg[8] = value; //Value

  //All done
  return (true);
}

//Add another keyID and value to an existing UBX-CFG-VALSET ubxPacket
//This function takes a full 32-bit key and 32-bit value
uint8_t addCfgValset32(uint32_t key, uint32_t value)
{
  //Load key into outgoing payload
  payloadCfg[packetCfg.len + 0] = key >> 8 * 0; //Key LSB
  payloadCfg[packetCfg.len + 1] = key >> 8 * 1;
  payloadCfg[packetCfg.len + 2] = key >> 8 * 2;
  payloadCfg[packetCfg.len + 3] = key >> 8 * 3;

  //Load user's value
  payloadCfg[packetCfg.len + 4] = value >> 8 * 0; //Value LSB
  payloadCfg[packetCfg.len + 5] = value >> 8 * 1;
  payloadCfg[packetCfg.len + 6] = value >> 8 * 2;
  payloadCfg[packetCfg.len + 7] = value >> 8 * 3;

  //Update packet length: 4 byte key ID, 4 bytes of value
  packetCfg.len = packetCfg.len + 4 + 4;

  //All done
  return (true);
}

//Add another keyID and value to an existing UBX-CFG-VALSET ubxPacket
//This function takes a full 32-bit key and 16-bit value
uint8_t addCfgValset16(uint32_t key, uint16_t value)
{
  //Load key into outgoing payload
  payloadCfg[packetCfg.len + 0] = key >> 8 * 0; //Key LSB
  payloadCfg[packetCfg.len + 1] = key >> 8 * 1;
  payloadCfg[packetCfg.len + 2] = key >> 8 * 2;
  payloadCfg[packetCfg.len + 3] = key >> 8 * 3;

  //Load user's value
  payloadCfg[packetCfg.len + 4] = value >> 8 * 0; //Value LSB
  payloadCfg[packetCfg.len + 5] = value >> 8 * 1;

  //Update packet length: 4 byte key ID, 2 bytes of value
  packetCfg.len = packetCfg.len + 4 + 2;

  //All done
  return (true);
}

//Add another keyID and value to an existing UBX-CFG-VALSET ubxPacket
//This function takes a full 32-bit key and 8-bit value
uint8_t addCfgValset8(uint32_t key, uint8_t value)
{
  //Load key into outgoing payload
  payloadCfg[packetCfg.len + 0] = key >> 8 * 0; //Key LSB
  payloadCfg[packetCfg.len + 1] = key >> 8 * 1;
  payloadCfg[packetCfg.len + 2] = key >> 8 * 2;
  payloadCfg[packetCfg.len + 3] = key >> 8 * 3;

  //Load user's value
  payloadCfg[packetCfg.len + 4] = value; //Value

  //Update packet length: 4 byte key ID, 1 byte value
  packetCfg.len = packetCfg.len + 4 + 1;

  //All done
  return (true);
}

//Add a final keyID and value to an existing UBX-CFG-VALSET ubxPacket and send it
//This function takes a full 32-bit key and 32-bit value
uint8_t sendCfgValset32(uint32_t key, uint32_t value, uint16_t maxWait)
{
  //Load keyID and value into outgoing payload
  addCfgValset32(key, value);

  //Send VALSET command with this key and value
  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Add a final keyID and value to an existing UBX-CFG-VALSET ubxPacket and send it
//This function takes a full 32-bit key and 16-bit value
uint8_t sendCfgValset16(uint32_t key, uint16_t value, uint16_t maxWait)
{
  //Load keyID and value into outgoing payload
  addCfgValset16(key, value);

  //Send VALSET command with this key and value
  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Add a final keyID and value to an existing UBX-CFG-VALSET ubxPacket and send it
//This function takes a full 32-bit key and 8-bit value
uint8_t sendCfgValset8(uint32_t key, uint8_t value, uint16_t maxWait)
{
  //Load keyID and value into outgoing payload
  addCfgValset8(key, value);

  //Send VALSET command with this key and value
  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Get the current TimeMode3 settings - these contain survey in statuses
bool getSurveyMode(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_TMODE3;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_RECEIVED); // We are expecting data and an ACK
}

//Control Survey-In for NEO-M8P
bool setSurveyMode(uint8_t mode, uint16_t observationTime, float requiredAccuracy, uint16_t maxWait)
{
  if (getSurveyMode(maxWait) == false) //Ask module for the current TimeMode3 settings. Loads into payloadCfg.
    return (false);

  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_TMODE3;
  packetCfg.len = 40;
  packetCfg.startingSpot = 0;

  //Clear packet payload
  for (uint8_t x = 0; x < packetCfg.len; x++)
    packetCfg.payload[x] = 0;

  //payloadCfg should be loaded with poll response. Now modify only the bits we care about
  payloadCfg[2] = mode; //Set mode. Survey-In and Disabled are most common. Use ECEF (not LAT/LON/ALT).

  //svinMinDur is U4 (uint32_t) but we'll only use a uint16_t (waiting more than 65535 seconds seems excessive!)
  payloadCfg[24] = observationTime & 0xFF; //svinMinDur in seconds
  payloadCfg[25] = observationTime >> 8;   //svinMinDur in seconds
  payloadCfg[26] = 0;                      //Truncate to 16 bits
  payloadCfg[27] = 0;                      //Truncate to 16 bits

  //svinAccLimit is U4 (uint32_t) in 0.1mm.
  uint32_t svinAccLimit = (uint32_t)(requiredAccuracy * 10000.0); //Convert m to 0.1mm
  payloadCfg[28] = svinAccLimit & 0xFF;                           //svinAccLimit in 0.1mm increments
  payloadCfg[29] = svinAccLimit >> 8;
  payloadCfg[30] = svinAccLimit >> 16;
  payloadCfg[31] = svinAccLimit >> 24;

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Begin Survey-In for NEO-M8P
bool enableSurveyMode(uint16_t observationTime, float requiredAccuracy, uint16_t maxWait)
{
  return (setSurveyMode(SVIN_MODE_ENABLE, observationTime, requiredAccuracy, maxWait));
}

//Stop Survey-In for NEO-M8P
bool disableSurveyMode(uint16_t maxWait)
{
  return (setSurveyMode(SVIN_MODE_DISABLE, 0, 0, maxWait));
}

//Reads survey in status and sets the global variables
//for status, position valid, observation time, and mean 3D StdDev
//Returns true if commands was successful
bool getSurveyStatus(uint16_t maxWait)
{
  //Reset variables
  svin.active = false;
  svin.valid = false;
  svin.observationTime = 0;
  svin.meanAccuracy = 0;

  packetCfg.cls = UBX_CLASS_NAV;
  packetCfg.id = UBX_NAV_SVIN;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  if ((sendCommand(&packetCfg, maxWait)) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (false);                                                         //If command send fails then bail

  //We got a response, now parse the bits into the svin structure

  //dur (Passed survey-in observation time) is U4 (uint32_t) seconds. We truncate to 16 bits
  //(waiting more than 65535 seconds (18.2 hours) seems excessive!)
  uint32_t tmpObsTime = extractLong(8);
  if (tmpObsTime <= 0xFFFF)
  {
    svin.observationTime = (uint16_t)tmpObsTime;
  }
  else
  {
    svin.observationTime = 0xFFFF;
  }

  // meanAcc is U4 (uint32_t) in 0.1mm. We convert this to float.
  uint32_t tempFloat = extractLong(28);
  svin.meanAccuracy = ((float)tempFloat) / 10000.0; //Convert 0.1mm to m

  svin.valid = payloadCfg[36];  //1 if survey-in position is valid, 0 otherwise
  svin.active = payloadCfg[37]; //1 if survey-in in progress, 0 otherwise

  return (true);
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
bool setUART2Output(uint8_t comSettings, uint16_t maxWait)
{
  return (setPortOutput(COM_PORT_UART2, comSettings, maxWait));
}
bool setUSBOutput(uint8_t comSettings, uint16_t maxWait)
{
  return (setPortOutput(COM_PORT_USB, comSettings, maxWait));
}
bool setSPIOutput(uint8_t comSettings, uint16_t maxWait)
{
  return (setPortOutput(COM_PORT_SPI, comSettings, maxWait));
}

//Set the rate at which the module will give us an updated navigation solution
//Expects a number that is the updates per second. For example 1 = 1Hz, 2 = 2Hz, etc.
//Max is 40Hz(?!)
bool setNavigationFrequency(uint8_t navFreq, uint16_t maxWait)
{
  //iupdateRate > 40) updateRate = 40; //Not needed: module will correct out of bounds values

  //Adjust the I2C polling timeout based on update rate
  i2cPollingWait = 1000 / (navFreq * 4); //This is the number of ms to wait between checks for new I2C data

  //Query the module for the latest lat/long
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RATE;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //This will load the payloadCfg array with current settings of the given register
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (false);                                                       //If command send fails then bail

  uint16_t measurementRate = 1000 / navFreq;

  //payloadCfg is now loaded with current bytes. Change only the ones we need to
  payloadCfg[0] = measurementRate & 0xFF; //measRate LSB
  payloadCfg[1] = measurementRate >> 8;   //measRate MSB

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Get the rate at which the module is outputting nav solutions
uint8_t getNavigationFrequency(uint16_t maxWait)
{
  //Query the module for the latest lat/long
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RATE;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //This will load the payloadCfg array with current settings of the given register
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (false);                                                       //If command send fails then bail

  uint16_t measurementRate = 0;

  //payloadCfg is now loaded with current bytes. Get what we need
  measurementRate = extractInt(0); //Pull from payloadCfg at measRate LSB

  measurementRate = 1000 / measurementRate; //This may return an int when it's a float, but I'd rather not return 4 bytes
  return (measurementRate);
}

//In case no config access to the GPS is possible and PVT is send cyclically already
//set config to suitable parameters
bool assumeAutoPVT(bool enabled, bool implicitUpdate)
{
  bool changes = autoPVT != enabled || autoPVTImplicitUpdate != implicitUpdate;
  if (changes)
  {
    autoPVT = enabled;
    autoPVTImplicitUpdate = implicitUpdate;
  }
  return changes;
}


//Configure a given message type for a given port (UART1, I2C, SPI, etc)
bool configureMessage(uint8_t msgClass, uint8_t msgID, uint8_t portID, uint8_t sendRate, uint16_t maxWait)
{
  //Poll for the current settings for a given message
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_MSG;
  packetCfg.len = 2;
  packetCfg.startingSpot = 0;

  payloadCfg[0] = msgClass;
  payloadCfg[1] = msgID;

  //This will load the payloadCfg array with current settings of the given register
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (false);                                                       //If command send fails then bail

  //Now send it back with new mods
  packetCfg.len = 8;

  //payloadCfg is now loaded with current bytes. Change only the ones we need to
  payloadCfg[2 + portID] = sendRate; //Send rate is relative to the event a message is registered on. For example, if the rate of a navigation message is set to 2, the message is sent every 2nd navigation solution.

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

//Enable a given message type, default of 1 per update rate (usually 1 per second)
bool enableMessage(uint8_t msgClass, uint8_t msgID, uint8_t portID, uint8_t rate, uint16_t maxWait)
{
  return (configureMessage(msgClass, msgID, portID, rate, maxWait));
}
//Disable a given message type on a given port
bool disableMessage(uint8_t msgClass, uint8_t msgID, uint8_t portID, uint16_t maxWait)
{
  return (configureMessage(msgClass, msgID, portID, 0, maxWait));
}

bool enableNMEAMessage(uint8_t msgID, uint8_t portID, uint8_t rate, uint16_t maxWait)
{
  return (configureMessage(UBX_CLASS_NMEA, msgID, portID, rate, maxWait));
}
bool disableNMEAMessage(uint8_t msgID, uint8_t portID, uint16_t maxWait)
{
  return (enableNMEAMessage(msgID, portID, 0, maxWait));
}

//Given a message number turns on a message ID for output over a given portID (UART, I2C, SPI, USB, etc)
//To disable a message, set secondsBetween messages to 0
//Note: This function will return false if the message is already enabled
//For base station RTK output we need to enable various sentences

//NEO-M8P has four:
//1005 = 0xF5 0x05 - Stationary RTK reference ARP
//1077 = 0xF5 0x4D - GPS MSM7
//1087 = 0xF5 0x57 - GLONASS MSM7
//1230 = 0xF5 0xE6 - GLONASS code-phase biases, set to once every 10 seconds

//ZED-F9P has six:
//1005, 1074, 1084, 1094, 1124, 1230

//Much of this configuration is not documented and instead discerned from u-center binary console
bool enableRTCMmessage(uint8_t messageNumber, uint8_t portID, uint8_t sendRate, uint16_t maxWait)
{
  return (configureMessage(UBX_RTCM_MSB, messageNumber, portID, sendRate, maxWait));
}

//Disable a given message on a given port by setting secondsBetweenMessages to zero
bool disableRTCMmessage(uint8_t messageNumber, uint8_t portID, uint16_t maxWait)
{
  return (enableRTCMmessage(messageNumber, portID, 0, maxWait));
}



//Clear the antenna control settings using UBX-CFG-ANT
//This function is hopefully redundant but may be needed to release
//any PIO pins pre-allocated for antenna functions
bool clearAntPIO(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_ANT;
  packetCfg.len = 4;
  packetCfg.startingSpot = 0;

  payloadCfg[0] = 0x10; // Antenna flag mask: set the recovery bit
  payloadCfg[1] = 0;
  payloadCfg[2] = 0xFF; // Antenna pin configuration: set pinSwitch and pinSCD to 31
  payloadCfg[3] = 0xFF; // Antenna pin configuration: set pinOCD to 31, set reconfig bit

  return ((sendCommand(&packetCfg, maxWait)) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}


//Power Save Mode
//Enables/Disables Low Power Mode using UBX-CFG-RXM
bool powerSaveMode(bool power_save, uint16_t maxWait)
{
  // Let's begin by checking the Protocol Version as UBX_CFG_RXM is not supported on the ZED (protocol >= 27)
  uint8_t protVer = getProtocolVersionHigh(maxWait);
  /*
  if (_printDebug == true)
  {
    printf("Protocol version is ");
    printf(protVer);
  }
  */
  if (protVer >= 27)
  {
    if (_printDebug == true)
    {
      printf("powerSaveMode (UBX-CFG-RXM) is not supported by this protocol version");
    }
    return (false);
  }

  // Now let's change the power setting using UBX-CFG-RXM
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RXM;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //Ask module for the current power management settings. Loads into payloadCfg.
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (false);

  if (power_save)
  {
    payloadCfg[1] = 1; // Power Save Mode
  }
  else
  {
    payloadCfg[1] = 0; // Continuous Mode
  }

  packetCfg.len = 2;
  packetCfg.startingSpot = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_SENT); // We are only expecting an ACK
}

// Get Power Save Mode
// Returns the current Low Power Mode using UBX-CFG-RXM
// Returns 255 if the sendCommand fails
uint8_t getPowerSaveMode(uint16_t maxWait)
{
  // Let's begin by checking the Protocol Version as UBX_CFG_RXM is not supported on the ZED (protocol >= 27)
  uint8_t protVer = getProtocolVersionHigh(maxWait);
  /*
  if (_printDebug == true)
  {
    printf("Protocol version is ");
    printf(protVer);
  }
  */
  if (protVer >= 27)
  {
    if (_printDebug == true)
    {
      printf("powerSaveMode (UBX-CFG-RXM) is not supported by this protocol version");
    }
    return (255);
  }

  // Now let's read the power setting using UBX-CFG-RXM
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_RXM;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;

  //Ask module for the current power management settings. Loads into payloadCfg.
  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are expecting data and an ACK
    return (255);

  return (payloadCfg[1]); // Return the low power mode
}


// Set the power saving config Setting using UBX-CFG-PM2
// This is the original packet that was sent.
//static uint8_t powersave_config[]= 
//{
//	0xB5 ,0x62 ,0x06 ,0x3B ,0x30 ,0x00 /* UBX-CFG-PM2 */
//	,0x02 ,0x06 ,0x00 ,0x00 /* v2, reserved 1..3 */
//	,0x60 ,0x90 ,0x40 ,0x01 /* ON/OFF tracking mode, update ephemeris */
//	,0x00 ,0x00 ,0x00 ,0x00 /* update period, 0 ms(infinity) */
//	,0x00 ,0x00 ,0x00 ,0x00 /* search period, 0 ms(infinity) */
//	,0x00 ,0x00 ,0x00 ,0x00 /* grid offset */
//	,0x02 ,0x00             /* on-time after first fix */
//	,0x00 ,0x00             /* minimum acquisition time */
//	,0x2C ,0x01 ,0x00 ,0x00
//	,0x4F ,0xC1 ,0x03 ,0x00
//	,0x87 ,0x02 ,0x00 ,0x00
//	,0xFF ,0x00 ,0x00 ,0x00
//	,0x64 ,0x40 ,0x01 ,0x00
//	,0x00 ,0x00 ,0x00 ,0x00
//	,0x19 ,0xB8

//};

bool set_powersave_config(uint16_t maxWait)
{
  packetCfg.cls = UBX_CLASS_CFG;
  packetCfg.id = UBX_CFG_PM2;
  packetCfg.len = 0;
  packetCfg.startingSpot = 0;


//	,0x02 ,0x06 ,0x00 ,0x00 /* v2, reserved 1..3 */
  payloadCfg[packetCfg.len++] = 0x02;            // 
  payloadCfg[packetCfg.len++] = 0x06;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	

//	,0x60 ,0x90 ,0x40 ,0x01 /* ON/OFF tracking mode, update ephemeris */
  payloadCfg[packetCfg.len++] = 0x60;            // 
  payloadCfg[packetCfg.len++] = 0x90;            // 
  payloadCfg[packetCfg.len++] = 0x40;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 

//	,0x00 ,0x00 ,0x00 ,0x00 /* update period, 0 ms(infinity) */

  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	
	//	,0x00 ,0x00 ,0x00 ,0x00 /* search period, 0 ms(infinity) */

  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	
//	,0x00 ,0x00 ,0x00 ,0x00 /* grid offset */

  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  
//	,0x02 ,0x00             /* on-time after first fix */
  payloadCfg[packetCfg.len++] = 0x02;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 

//	,0x00 ,0x00             /* minimum acquisition time */
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	
	//	,0x2C ,0x01 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0x2C;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 

//	,0x4F ,0xC1 ,0x03 ,0x00

  payloadCfg[packetCfg.len++] = 0x4F;            // 
  payloadCfg[packetCfg.len++] = 0xC1;            // 
  payloadCfg[packetCfg.len++] = 0x03;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	
	//	,0x87 ,0x02 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0x87;            // 
  payloadCfg[packetCfg.len++] = 0x02;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 

//	,0xFF ,0x00 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0xFF;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	
	//	,0x64 ,0x40 ,0x01 ,0x00

  payloadCfg[packetCfg.len++] = 0x64;            // 
  payloadCfg[packetCfg.len++] = 0x40;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 

//	,0x00 ,0x00 ,0x00 ,0x00

  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
	
	
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
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 


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
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 


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
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x20;            // 
  payloadCfg[packetCfg.len++] = 0x07;            // 
	
	//		0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,    /* GPS enable */

	/* GPS enable */
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x10;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
	payloadCfg[packetCfg.len++] = 0x01;            //
	
	//		0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,	/* SBAS disable */

	/* SBAS disable */
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x03;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  
	//		0x02,0x04,0x08,0x00,0x00,0x00,0x01,0x01,	/* Galileo disable */
	/* Galileo disable */
  payloadCfg[packetCfg.len++] = 0x02;            // 
  payloadCfg[packetCfg.len++] = 0x04;            // 
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 

//		0x03,0x08,0x10,0x00,0x00,0x00,0x01,0x01,	/* Beidou disable */
	/* Beidou disable */
  payloadCfg[packetCfg.len++] = 0x03;            // 
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x10;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 

//		0x04,0x00,0x08,0x00,0x00,0x00,0x01,0x01,	/* IMES disable */
	/* IMES disable */
  payloadCfg[packetCfg.len++] = 0x04;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 

//		0x05,0x00,0x03,0x00,0x01,0x00,0x01,0x01,	/* QZSS enable */
	/* QZSS enable */
  payloadCfg[packetCfg.len++] = 0x05;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x03;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 

//		0x06,0x08,0x0E,0x00,0x00,0x00,0x01,0x01,	/* GLONASS disable */

	/* GLONASS disable */
  payloadCfg[packetCfg.len++] = 0x06;            // 
  payloadCfg[packetCfg.len++] = 0x08;            // 
  payloadCfg[packetCfg.len++] = 0x0E;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x00;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
  payloadCfg[packetCfg.len++] = 0x01;            // 
	


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
      printf("%s",statusString(retVal));
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

int32_t getHighResLatitude(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.highResLatitude == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.highResLatitude = false; //Since we are about to give this to user, mark this data as stale
  return (highResLatitude);
}

int8_t getHighResLatitudeHp(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.highResLatitudeHp == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.highResLatitudeHp = false; //Since we are about to give this to user, mark this data as stale
  return (highResLatitudeHp);
}

int32_t getHighResLongitude(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.highResLongitude == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.highResLongitude = false; //Since we are about to give this to user, mark this data as stale
  return (highResLongitude);
}

int8_t getHighResLongitudeHp(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.highResLongitudeHp == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.highResLongitudeHp = false; //Since we are about to give this to user, mark this data as stale
  return (highResLongitudeHp);
}

int32_t getElipsoid(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.elipsoid == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.elipsoid = false; //Since we are about to give this to user, mark this data as stale
  return (elipsoid);
}

int8_t getElipsoidHp(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.elipsoidHp == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.elipsoidHp = false; //Since we are about to give this to user, mark this data as stale
  return (elipsoidHp);
}

int32_t getMeanSeaLevel(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.meanSeaLevel == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.meanSeaLevel = false; //Since we are about to give this to user, mark this data as stale
  return (meanSeaLevel);
}

int8_t getMeanSeaLevelHp(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.meanSeaLevelHp == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.meanSeaLevelHp = false; //Since we are about to give this to user, mark this data as stale
  return (meanSeaLevelHp);
}

// getGeoidSeparation is currently redundant. The geoid separation seems to only be provided in NMEA GGA and GNS messages.
int32_t getGeoidSeparation(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.geoidSeparation == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.geoidSeparation = false; //Since we are about to give this to user, mark this data as stale
  return (geoidSeparation);
}

uint32_t getHorizontalAccuracy(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.horizontalAccuracy == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.horizontalAccuracy = false; //Since we are about to give this to user, mark this data as stale
  return (horizontalAccuracy);
}

uint32_t getVerticalAccuracy(uint16_t maxWait /* = 250*/)
{
  if (highResModuleQueried.verticalAccuracy == false)
    getHPPOSLLH(maxWait);
  highResModuleQueried.verticalAccuracy = false; //Since we are about to give this to user, mark this data as stale
  return (verticalAccuracy);
}

bool getHPPOSLLH(uint16_t maxWait)
{
  //The GPS is not automatically reporting navigation position so we have to poll explicitly
  packetCfg.cls = UBX_CLASS_NAV;
  packetCfg.id = UBX_NAV_HPPOSLLH;
  packetCfg.len = 0;

  return (sendCommand(&packetCfg, maxWait) == SFE_UBLOX_STATUS_DATA_RECEIVED); // We are only expecting data (no ACK)
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

//Get the carrier phase range solution status
//Useful when querying module to see if it has high-precision RTK fix
//0=No solution, 1=Float solution, 2=Fixed solution
uint8_t getCarrierSolutionType(uint16_t maxWait)
{
  if (moduleQueried.carrierSolution == false)
    getPVT(maxWait);
  moduleQueried.carrierSolution = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (carrierSolution);
}

//Get the ground speed in mm/s
int32_t getGroundSpeed(uint16_t maxWait)
{
  if (moduleQueried.groundSpeed == false)
    getPVT(maxWait);
  moduleQueried.groundSpeed = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (groundSpeed);
}

//Get the heading of motion (as opposed to heading of car) in degrees * 10^-5
int32_t getHeading(uint16_t maxWait)
{
  if (moduleQueried.headingOfMotion == false)
    getPVT(maxWait);
  moduleQueried.headingOfMotion = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (headingOfMotion);
}

//Get the positional dillution of precision * 10^-2
uint16_t getPDOP(uint16_t maxWait)
{
  if (moduleQueried.pDOP == false)
    getPVT(maxWait);
  moduleQueried.pDOP = false; //Since we are about to give this to user, mark this data as stale
  moduleQueried.all = false;

  return (pDOP);
}

//Get the current protocol version of the Ublox module we're communicating with
//This is helpful when deciding if we should call the high-precision Lat/Long (HPPOSLLH) or the regular (POSLLH)
uint8_t getProtocolVersionHigh(uint16_t maxWait)
{
  if (moduleQueried.versionNumber == false)
    getProtocolVersion(maxWait);
  return (versionHigh);
}

//Get the current protocol version of the Ublox module we're communicating with
//This is helpful when deciding if we should call the high-precision Lat/Long (HPPOSLLH) or the regular (POSLLH)
uint8_t getProtocolVersionLow(uint16_t maxWait)
{
  if (moduleQueried.versionNumber == false)
    getProtocolVersion(maxWait);
  return (versionLow);
}

//Get the current protocol version of the Ublox module we're communicating with
//This is helpful when deciding if we should call the high-precision Lat/Long (HPPOSLLH) or the regular (POSLLH)
bool getProtocolVersion(uint16_t maxWait)
{
  //Send packet with only CLS and ID, length of zero. This will cause the module to respond with the contents of that CLS/ID.
  packetCfg.cls = UBX_CLASS_MON;
  packetCfg.id = UBX_MON_VER;

  packetCfg.len = 0;
  packetCfg.startingSpot = 40; //Start at first "extended software information" string

  if (sendCommand(&packetCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED) // We are only expecting data (no ACK)
    return (false);                                                       //If command send fails then bail

  //Payload should now contain ~220 characters (depends on module type)

  // if (_printDebug == true)
  // {
  //   printf("MON VER Payload:");
  //   for (int location = 0; location < packetCfg.len; location++)
  //   {
  //     if (location % 30 == 0)
  //       printf();
  //     write(payloadCfg[location]);
  //   }
  //   printf();
  // }

  //We will step through the payload looking at each extension field of 30 bytes
  for (uint8_t extensionNumber = 0; extensionNumber < 10; extensionNumber++)
  {
    //Now we need to find "PROTVER=18.00" in the incoming byte stream
    if (payloadCfg[(30 * extensionNumber) + 0] == 'P' && payloadCfg[(30 * extensionNumber) + 6] == 'R')
    {
      versionHigh = (payloadCfg[(30 * extensionNumber) + 8] - '0') * 10 + (payloadCfg[(30 * extensionNumber) + 9] - '0');  //Convert '18' to 18
      versionLow = (payloadCfg[(30 * extensionNumber) + 11] - '0') * 10 + (payloadCfg[(30 * extensionNumber) + 12] - '0'); //Convert '00' to 00
      moduleQueried.versionNumber = true;                                                                                  //Mark this data as new

      if (_printDebug == true)
      {
        printf("Protocol version: ");
        printf("%d",versionHigh);
        printf(".");
        printf("%d\r\n",versionLow);
      }
      return (true); //Success!
    }
  }

  return (false); //We failed
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


#endif //#if 0

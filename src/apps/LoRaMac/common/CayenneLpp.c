/*!
 * \file  CayenneLpp.c
 *
 * \brief Implements the Cayenne Low Power Protocol
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>

#include "utilities.h"
#include "CayenneLpp.h"

#define CAYENNE_LPP_MAXBUFFER_SIZE                  242

static uint8_t CayenneLppBuffer[CAYENNE_LPP_MAXBUFFER_SIZE];
static uint8_t CayenneLppCursor = 0;

void CayenneLppInit( void )
{
    CayenneLppCursor = 0;
}

void CayenneLppReset( void )
{
    CayenneLppCursor = 0;
}

uint8_t CayenneLppGetSize( void )
{
    return CayenneLppCursor;
}

uint8_t* CayenneLppGetBuffer( void )
{
    return CayenneLppBuffer;
}

uint8_t CayenneLppCopy( uint8_t* dst )
{
    memcpy1( dst, CayenneLppBuffer, CayenneLppCursor );

    return CayenneLppCursor;
}


uint8_t CayenneLppAddDigitalInput( uint8_t channel, uint8_t value )
{
    if( ( CayenneLppCursor + LPP_DIGITAL_INPUT_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_DIGITAL_INPUT; 
    CayenneLppBuffer[CayenneLppCursor++] = value; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddDigitalOutput( uint8_t channel, uint8_t value )
{
    if( ( CayenneLppCursor + LPP_DIGITAL_OUTPUT_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_DIGITAL_OUTPUT; 
    CayenneLppBuffer[CayenneLppCursor++] = value; 

    return CayenneLppCursor;
}


uint8_t CayenneLppAddAnalogInput( uint8_t channel, float value )
{
    if( ( CayenneLppCursor + LPP_ANALOG_INPUT_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }

    int16_t val = ( int16_t ) ( value * 100 );
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_ANALOG_INPUT; 
    CayenneLppBuffer[CayenneLppCursor++] = val >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = val; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddAnalogOutput( uint8_t channel, float value )
{
    if( ( CayenneLppCursor + LPP_ANALOG_OUTPUT_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t val = ( int16_t ) ( value * 100 );
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_ANALOG_OUTPUT;
    CayenneLppBuffer[CayenneLppCursor++] = val >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = val; 

    return CayenneLppCursor;
}


uint8_t CayenneLppAddLuminosity( uint8_t channel, uint16_t lux )
{
    if( ( CayenneLppCursor + LPP_LUMINOSITY_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_LUMINOSITY; 
    CayenneLppBuffer[CayenneLppCursor++] = lux >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = lux; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddPresence( uint8_t channel, uint8_t value )
{
    if( ( CayenneLppCursor + LPP_PRESENCE_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_PRESENCE; 
    CayenneLppBuffer[CayenneLppCursor++] = value; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddTemperature( uint8_t channel, float celsius )
{
    if( ( CayenneLppCursor + LPP_TEMPERATURE_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t val = ( int16_t) ( celsius * 10 );
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_TEMPERATURE; 
    CayenneLppBuffer[CayenneLppCursor++] = val >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = val; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddRelativeHumidity( uint8_t channel, float rh )
{
    if( ( CayenneLppCursor + LPP_RELATIVE_HUMIDITY_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_RELATIVE_HUMIDITY; 
    CayenneLppBuffer[CayenneLppCursor++] = (uint8_t ) ( rh * 2 ); 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddAccelerometer( uint8_t channel, float x, float y, float z )
{
    if( ( CayenneLppCursor + LPP_ACCELEROMETER_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t vx = ( int16_t ) ( x * 1000 );
    int16_t vy = ( int16_t ) ( y * 1000 );
    int16_t vz = ( int16_t ) ( z * 1000 );

    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_ACCELEROMETER; 
    CayenneLppBuffer[CayenneLppCursor++] = vx >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = vx; 
    CayenneLppBuffer[CayenneLppCursor++] = vy >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = vy; 
    CayenneLppBuffer[CayenneLppCursor++] = vz >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = vz; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddBarometricPressure( uint8_t channel, float hpa )
{
    if( ( CayenneLppCursor + LPP_BAROMETRIC_PRESSURE_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t val = ( int16_t ) ( hpa * 10 );

    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_BAROMETRIC_PRESSURE; 
    CayenneLppBuffer[CayenneLppCursor++] = val >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = val; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddGyrometer( uint8_t channel, float x, float y, float z )
{
    if( ( CayenneLppCursor + LPP_GYROMETER_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int16_t vx = ( int16_t ) ( x * 100 );
    int16_t vy = ( int16_t ) ( y * 100 );
    int16_t vz = ( int16_t ) ( z * 100 );

    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_GYROMETER; 
    CayenneLppBuffer[CayenneLppCursor++] = vx >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = vx; 
    CayenneLppBuffer[CayenneLppCursor++] = vy >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = vy; 
    CayenneLppBuffer[CayenneLppCursor++] = vz >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = vz; 

    return CayenneLppCursor;
}

uint8_t CayenneLppAddGps( uint8_t channel, float latitude, float longitude, float meters )
{
    if( ( CayenneLppCursor + LPP_GPS_SIZE ) > CAYENNE_LPP_MAXBUFFER_SIZE )
    {
        return 0;
    }
    int32_t lat = ( int32_t ) ( latitude * 10000 );
    int32_t lon = ( int32_t ) ( longitude * 10000 );
    int32_t alt = ( int32_t ) ( meters * 100 );

    CayenneLppBuffer[CayenneLppCursor++] = channel; 
    CayenneLppBuffer[CayenneLppCursor++] = LPP_GPS; 

    CayenneLppBuffer[CayenneLppCursor++] = lat >> 16; 
    CayenneLppBuffer[CayenneLppCursor++] = lat >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = lat; 
    CayenneLppBuffer[CayenneLppCursor++] = lon >> 16; 
    CayenneLppBuffer[CayenneLppCursor++] = lon >> 8; 
    CayenneLppBuffer[CayenneLppCursor++] = lon; 
    CayenneLppBuffer[CayenneLppCursor++] = alt >> 16; 
    CayenneLppBuffer[CayenneLppCursor++] = alt >> 8;
    CayenneLppBuffer[CayenneLppCursor++] = alt;

    return CayenneLppCursor;
}

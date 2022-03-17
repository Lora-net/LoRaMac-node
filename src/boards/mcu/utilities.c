/*!
 * \file      utilities.h
 *
 * \brief     Helper functions implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <stdlib.h>
#include <stdio.h>
#include "utilities.h"

/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

void srand1( uint32_t seed )
{
    next = seed;
}
// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    while( size-- )
    {
        *dst++ = *src++;
    }
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
    while( size-- )
    {
        *dst++ = value;
    }
}

int8_t Nibble2HexChar( uint8_t a )
{
    if( a < 10 )
    {
        return '0' + a;
    }
    else if( a < 16 )
    {
        return 'A' + ( a - 10 );
    }
    else
    {
        return '?';
    }
}

uint32_t Crc32( uint8_t *buffer, uint16_t length )
{
    // The CRC calculation follows CCITT - 0x04C11DB7
    const uint32_t reversedPolynom = 0xEDB88320;

    // CRC initial value
    uint32_t crc = 0xFFFFFFFF;

    if( buffer == NULL )
    {
        return 0;
    }

    for( uint16_t i = 0; i < length; ++i )
    {
        crc ^= ( uint32_t )buffer[i];
        for( uint16_t i = 0; i < 8; i++ )
        {
            crc = ( crc >> 1 ) ^ ( reversedPolynom & ~( ( crc & 0x01 ) - 1 ) );
        }
    }

    return ~crc;
}

uint32_t Crc32Init( void )
{
    return 0xFFFFFFFF;
}

uint32_t Crc32Update( uint32_t crcInit, uint8_t *buffer, uint16_t length )
{
    // The CRC calculation follows CCITT - 0x04C11DB7
    const uint32_t reversedPolynom = 0xEDB88320;

    // CRC initial value
    uint32_t crc = crcInit;

    if( buffer == NULL )
    {
        return 0;
    }

    for( uint16_t i = 0; i < length; ++i )
    {
        crc ^= ( uint32_t )buffer[i];
        for( uint16_t i = 0; i < 8; i++ )
        {
            crc = ( crc >> 1 ) ^ ( reversedPolynom & ~( ( crc & 0x01 ) - 1 ) );
        }
    }
    return crc;
}

uint32_t Crc32Finalize( uint32_t crc )
{
    return ~crc;
}

uint8_t ds_crc8_bit_update(uint8_t crc8, uint8_t bit)
{
    uint8_t crc8_base = (crc8 >> 1) & 0x73;
    uint8_t xor1 = (crc8 & 1) ^ bit;
    return crc8_base | ((((crc8 >> 3) & 1) ^ xor1) << 2) | ((((crc8 >> 4) & 1) ^ xor1) << 3) | (xor1 << 7);
}

uint8_t ds_crc8_byte_update(uint8_t crc8, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        uint8_t mix = (crc8 ^ byte) & 0x01;
        crc8 >>= 1;
        if (mix)
        {
            crc8 ^= 0x8C;
        }
        byte >>= 1;
    }
    return crc8;
}
/**
 * @brief CRC8 implementation taken from https://github.com/JamieMichie/inav-pinio/blob/cafd2c3b0d9e791ceed298ef95be4440e2c24b2d/src/main/drivers/1-wire/ds_crc.c
 * 
 * @param addr 
 * @param len 
 * @return uint8_t 
 */
uint8_t Crc8Update(uint8_t crcInit, uint8_t *addr, uint16_t len)
{
    uint8_t crc8 = crcInit;
    for (uint8_t i = 0; i < len; ++i)
    {
        crc8 = ds_crc8_byte_update(crc8, addr[i]);
    }
    return crc8;
}

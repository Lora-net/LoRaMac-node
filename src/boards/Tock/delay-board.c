/*!
 * \file      delay-board.c
 *
 * \brief     Target board delay implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 */
#include "delay-board.h"

#include "libtock/timer.h"

void DelayMsMcu( uint32_t ms )
{
    delay_ms( ms );
}

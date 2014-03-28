/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Implements a FIFO buffer

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdbool.h>
#include <stdint.h>

/*!
 * FIFO
 */
typedef struct sFifo
{
	uint16_t Begin;
	uint16_t End;
	uint8_t *Data;
    uint16_t Size;
}tFifo;

void FifoInit( tFifo *fifo, uint8_t *buffer, uint16_t size );

void FifoPush( tFifo *fifo, uint8_t data );

uint8_t FifoPop( tFifo *fifo );

void FifoFlush( tFifo *fifo );

bool IsFifoEmpty( tFifo *fifo );

bool IsFifoFull( tFifo *fifo );

#endif // __FIFO_H__

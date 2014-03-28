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
#include "fifo.h"

static uint16_t FifoNext( tFifo *fifo, uint16_t index )
{
	return ( index + 1 ) % fifo->Size;
}

void FifoInit( tFifo *fifo, uint8_t *buffer, uint16_t size )
{
	fifo->Begin = 0;
	fifo->End = 0;
	fifo->Data = buffer;
	fifo->Size = size;
}

void FifoPush( tFifo *fifo, uint8_t data )
{
	fifo->End = FifoNext( fifo, fifo->End );
	fifo->Data[fifo->End] = data;
}

uint8_t FifoPop( tFifo *fifo )
{
	uint8_t data = fifo->Data[FifoNext( fifo, fifo->Begin )];

	fifo->Begin = FifoNext( fifo, fifo->Begin );
	return data;
}

void FifoFlush( tFifo *fifo )
{
	fifo->Begin = 0;
	fifo->End = 0;
}

bool IsFifoEmpty( tFifo *fifo )
{
	return ( fifo->Begin == fifo->End );
}

bool IsFifoFull( tFifo *fifo )
{
	return ( FifoNext( fifo, fifo->End ) == fifo->Begin );
}

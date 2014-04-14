/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: FSK continuous wave

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Jiapeng Li
*/
#include <string.h>
#include "board.h"
#include "radio.h"

#define SYS_FREQUENCY			434000000
#define POWER					20

/**
 * Main application entry point.
 */
int main( void )
{
    bool isMaster = true;
    uint8_t i;

    // Target board initialisation
    BoardInitMcu( );

    Radio.Init( NULL );
    Radio.SetChannel( SYS_FREQUENCY );
    Radio.SetTxConfig( MODEM_FSK, POWER, 0, 0, 9600, 0, 8, false, true, false, 3000000 );
	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );

	while(1);
}

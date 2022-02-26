/*!
 * \file      cli.h
 *
 * \brief     Command Line Interface handling implementation
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
 *              (C)2013-2020 Semtech
 *
 * \endcode
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "NvmDataMgmt.h"
#include "cli.h"
#include "nvmm.h"
#include "bsp.h"

void CliProcess( Uart_t* uart )
{
    uint8_t data = 0;

    if( UartGetChar( uart, &data ) == 0 )
    {
        if( data == 'C' )
        { // Escape character has been received
            printf( "ESC + " );
            while( UartGetChar( uart, &data ) != 0 )
            {
            }
            printf( "%c\n", data );
            if( data == 'W' )
            {
                printf( "Wiping EEPROM. It will take a few seconds....\n" );

                // Wipe out the EEPROM fully
                if (EEPROM_Wipe(0, EEPROM_SIZE) == true)
                {
                    printf( "\n\nEEPROM wipe succeed\n" );
                }
                else
                {
                    printf( "\n\nEEPROM wipe failed\n" );
                }
                printf( "\n\nPLEASE RESET THE END-DEVICE\n\n" );
                while( 1 );
            }
        }
    }
}

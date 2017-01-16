/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Implements a generic ADC driver

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "adc-board.h"

/*!
 * Flag to indicates if the ADC is initialized
 */
static bool AdcInitialized = false;

void AdcInit( Adc_t *obj, PinNames adcInput )
{
    if( AdcInitialized == false )
    {
        AdcInitialized = true;

        AdcMcuInit( obj, adcInput );
        AdcMcuConfig( );
    }
}

void AdcDeInit( Adc_t *obj )
{
    AdcInitialized = false;
}

uint16_t AdcReadChannel( Adc_t *obj, uint32_t channel )
{
    if( AdcInitialized == true )
    {
        return AdcMcuReadChannel( obj, channel );
    }
    else
    {
        return 0;
    }
}

/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Hex coder selector driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __SELECTOR_H__
#define __SELECTOR_H__

/*!
 * Gets the current hex coder selector position
 *
 * \retval position Hex coder current position ( On error position = 255 )
 */
uint8_t SelectorGetValue( void );

#endif // __SELECTOR_H__

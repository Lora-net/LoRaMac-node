/*!
 * \file      buttons.h
 *
 * \brief     Buttons management
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
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdbool.h>

/*!
 * \brief Buttons ID enumeration
 */
typedef enum ButtonsId_e
{
    BUTTON_LEFT,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
}ButtonsId_t;

/*!
 * \brief Initializes the buttons handling
 */
void ButtonsInit( void );

/*!
 * \brief Returns the button ID current state
 *
 * \param id    Button ID
 *
 * \retval state Current button state
 *               [true: Pressed, false: Released]
 */
bool ButtonGetState( ButtonsId_t id );

#endif // __BUTTONS_H__

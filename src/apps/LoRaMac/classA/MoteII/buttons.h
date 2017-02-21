/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Buttons management.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

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

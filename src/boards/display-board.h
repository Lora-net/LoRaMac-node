/*!
 * \file  display-board.h
 *
 * \brief Target board OLED low level driver implementation
 *
 * \remarks   Some snippets of these drivers are based on the Adafruit_GFX library.
 *            https://github.com/adafruit/Adafruit-GFX-Library
 *            Please take a look at their LICENSE.TXT file.
 *            Copyright (c) 2012 Adafruit Industries. All rights reserved.
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __DISPLAY_BOARD_H__
#define __DISPLAY_BOARD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/*!
 * \brief Display colors enumeration
 */
typedef enum
{
    DISPLAY_BLACK,
    DISPLAY_WHITE,
    DISPLAY_INVERSE,
}DisplayColor_t;

/*!
 * \brief Initializes the display
 */
void DisplayInit( void );

/*!
 * \brief Resets the display
 */
void DisplayReset( void );

/*!
 * \brief Sends a command to the display
 *
 * \param cmd Command to be sent
 */
void DisplaySendCommand( uint8_t cmd );

/*!
 * \brief Sends a data buffer to the display
 *
 * \param buffer Buffer to be sent
 * \param size   Buffer size to be sent
 */
void DisplaySendData( uint8_t *buffer, uint16_t size );

/*!
 * \brief Enables the display
 */
void DisplayOn( void );

/*!
 * \brief Disables the display
 */
void DisplayOff( void );

/*!
 * \brief Clears the display
 */
void DisplayClear( void );

/*!
 * \brief Inverts colors of the display
 *
 * \param invert [true: invert, false: normal]
 */
void DisplayInvertColors( bool invert );

/*!
 * \brief Updates the display with MCU RAM copy
 */
void DisplayUpdate( void );

/*!
 * \brief Sets the cursor at coordinates (x,y)
 *
 * \param x   X coordinate
 * \param y   Y coordinate
 */
void DisplaySetCursor( int16_t x, int16_t y );

/*!
 * \brief Gets current X coordinate of the cursor
 *
 * \retval x   X coordinate
 */
int16_t DisplayGetCursorX( void );

/*!
 * \brief Gets current Y coordinate of the cursor
 *
 * \retval y   Y coordinate
 */
int16_t DisplayGetCursorY( void );

/*!
 * \brief Sets text size
 *
 * \param s New text size
 */
void DisplaySetTextSize( uint8_t s );

/*!
 * \brief Sets text color
 *
 * \param color New text color
 */
void DisplaySetTextColor( DisplayColor_t color );

/*!
 * \brief Sets foreground and background color
 *
 * \param fg Foreground color
 * \param bg Background color
 */
void DisplaySetFgAndBg( DisplayColor_t fg, DisplayColor_t bg );

/*!
 * \brief Enables/Disable text wrapping
 *
 * \param w [true: wrap ON, false: wrap OFF]
 */
void DisplaySetTextWrap( bool w );

/*!
 * \brief Gets current display rotation
 *
 * \retval rotation   Display rotation (Vertical/Horizontal)
 */
uint8_t DisplayGetRotation( void );

/*!
 * \brief Sets current display rotation
 *
 * \param x   Display rotation (Vertical/Horizontal)
 */
void DisplaySetRotation( uint8_t x );

/*!
 * \brief Draws a pixel of color at coordinates (x,y)
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param color Pixel color
 */
void DisplayDrawPixel( int16_t x, int16_t y, DisplayColor_t color );

/*!
 * \brief Draws a line starting at coordinates (x0,y0) ending at
 *        coordinates (x1,y1) of color
 *
 * \param x0     X0 coordinate
 * \param y0     Y0 coordinate
 * \param x1     X1 coordinate
 * \param y1     Y1 coordinate
 * \param color  Line color
 */
void DisplayDrawLine( int16_t x0, int16_t y0, int16_t x1, int16_t y1, DisplayColor_t color );

/*!
 * \brief Draws a vertical line starting at coordinates (x,y) with given height
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param h     Line height
 * \param color Line color
 */
void DisplayDrawVerticalLine( int16_t x, int16_t y, int16_t h, DisplayColor_t color );

/*!
 * \brief Draws an Horizontal line starting at coordinates (x,y) with given width
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param w     Line width
 * \param color Line color
 */
void DisplayDrawHorizontalLine( int16_t x, int16_t y, int16_t w, DisplayColor_t color );

/*!
 * \brief Draws a rectangle at coordinates (x,y) with given width and height
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param w     Line width
 * \param h     Line height
 * \param color Line color
 */
void DisplayDrawRect( int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor_t color );

/*!
 * \brief Draws a filled rectangle at coordinates (x,y) with given width and height
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param w     Line width
 * \param h     Line height
 * \param color Fill color
 */
void DisplayFillRect( int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor_t color );

/*!
 * \brief Fills all display with pixels of color
 *
 * \param color Fill color
 */
void DisplayFillScreen( DisplayColor_t color );

/*!
 * \brief Draws a triangle by giving the 3 vertices coordinates
 *
 * \param x0    X0 coordinate
 * \param y0    Y0 coordinate
 * \param x1    X1 coordinate
 * \param y1    Y1 coordinate
 * \param x2    X2 coordinate
 * \param y2    Y2 coordinate
 * \param color Line color
 */
void DisplayDrawTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, DisplayColor_t color );

/*!
 * \brief Draws a filled triangle by giving the 3 vertices coordinates
 *
 * \param x0    X0 coordinate
 * \param y0    Y0 coordinate
 * \param x1    X1 coordinate
 * \param y1    Y1 coordinate
 * \param x2    X2 coordinate
 * \param y2    Y2 coordinate
 * \param color Fill color
 */
void DisplayFillTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, DisplayColor_t color ) ;

/*!
 * \brief Draws a character at given coordinates
 *
 * \param x     X coordinate
 * \param y     Y coordinate
 * \param c     Character
 * \param color Character color
 * \param bg    Background color
 * \param size  Character size
 */
void DisplayDrawChar( int16_t x, int16_t y, unsigned char c, DisplayColor_t color, DisplayColor_t bg, uint8_t size );

/*!
 * \brief Display putc function. (Mimics standard C putc function)
 *
 * \param c     Character
 */
void DisplayPutc( uint8_t c );

/*!
 * \brief Sets cursor at line
 *
 * \param line  Line number
 */
void DisplaySetLine( uint8_t line );

/*!
 * \brief Display print function. Prints the given string
 */
void DisplayPrint( const char *string );

/*!
 * \brief Display printf function. (Mimics standard C printf function)
 */
void DisplayPrintf( const char *format, ... );

#ifdef __cplusplus
}
#endif

#endif // __DISPLAY_BOARD_H__

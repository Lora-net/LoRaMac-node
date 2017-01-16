/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Driver for the low level handling of OLED display.

\remarks     Some snippets of these drivers are based on the Adafruit_GFX library.
             https://github.com/adafruit/Adafruit-GFX-Library
             Please take a look at their LICENSE.TXT file.
             Copyright (c) 2012 Adafruit Industries. All rights reserved.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdarg.h>
#include "board.h"
#include "display-board.h"

/*!
 * Standard ASCII 5x7 font
 */
const unsigned char  FontStd[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
    0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
    0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
    0x18, 0x3C, 0x7E, 0x3C, 0x18,
    0x1C, 0x57, 0x7D, 0x57, 0x1C,
    0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
    0x00, 0x18, 0x3C, 0x18, 0x00,
    0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
    0x00, 0x18, 0x24, 0x18, 0x00,
    0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
    0x30, 0x48, 0x3A, 0x06, 0x0E,
    0x26, 0x29, 0x79, 0x29, 0x26,
    0x40, 0x7F, 0x05, 0x05, 0x07,
    0x40, 0x7F, 0x05, 0x25, 0x3F,
    0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
    0x7F, 0x3E, 0x1C, 0x1C, 0x08,
    0x08, 0x1C, 0x1C, 0x3E, 0x7F,
    0x14, 0x22, 0x7F, 0x22, 0x14,
    0x5F, 0x5F, 0x00, 0x5F, 0x5F,
    0x06, 0x09, 0x7F, 0x01, 0x7F,
    0x00, 0x66, 0x89, 0x95, 0x6A,
    0x60, 0x60, 0x60, 0x60, 0x60,
    0x94, 0xA2, 0xFF, 0xA2, 0x94,
    0x08, 0x04, 0x7E, 0x04, 0x08,
    0x10, 0x20, 0x7E, 0x20, 0x10,
    0x08, 0x08, 0x2A, 0x1C, 0x08,
    0x08, 0x1C, 0x2A, 0x08, 0x08,
    0x1E, 0x10, 0x10, 0x10, 0x10,
    0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
    0x30, 0x38, 0x3E, 0x38, 0x30,
    0x06, 0x0E, 0x3E, 0x0E, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x5F, 0x00, 0x00,
    0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x56, 0x20, 0x50,
    0x00, 0x08, 0x07, 0x03, 0x00,
    0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00,
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
    0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x80, 0x70, 0x30, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x60, 0x60, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02,
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46,
    0x21, 0x41, 0x49, 0x4D, 0x33,
    0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39,
    0x3C, 0x4A, 0x49, 0x49, 0x31,
    0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36,
    0x46, 0x49, 0x49, 0x29, 0x1E,
    0x00, 0x00, 0x14, 0x00, 0x00,
    0x00, 0x40, 0x34, 0x00, 0x00,
    0x00, 0x08, 0x14, 0x22, 0x41,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08,
    0x02, 0x01, 0x59, 0x09, 0x06,
    0x3E, 0x41, 0x5D, 0x59, 0x4E,
    0x7C, 0x12, 0x11, 0x12, 0x7C,
    0x7F, 0x49, 0x49, 0x49, 0x36,
    0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x49, 0x49, 0x49, 0x41,
    0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x41, 0x51, 0x73,
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01,
    0x7F, 0x08, 0x14, 0x22, 0x41,
    0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x1C, 0x02, 0x7F,
    0x7F, 0x04, 0x08, 0x10, 0x7F,
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06,
    0x3E, 0x41, 0x51, 0x21, 0x5E,
    0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32,
    0x03, 0x01, 0x7F, 0x01, 0x03,
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F,
    0x3F, 0x40, 0x38, 0x40, 0x3F,
    0x63, 0x14, 0x08, 0x14, 0x63,
    0x03, 0x04, 0x78, 0x04, 0x03,
    0x61, 0x59, 0x49, 0x4D, 0x43,
    0x00, 0x7F, 0x41, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20,
    0x00, 0x41, 0x41, 0x41, 0x7F,
    0x04, 0x02, 0x01, 0x02, 0x04,
    0x40, 0x40, 0x40, 0x40, 0x40,
    0x00, 0x03, 0x07, 0x08, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x40,
    0x7F, 0x28, 0x44, 0x44, 0x38,
    0x38, 0x44, 0x44, 0x44, 0x28,
    0x38, 0x44, 0x44, 0x28, 0x7F,
    0x38, 0x54, 0x54, 0x54, 0x18,
    0x00, 0x08, 0x7E, 0x09, 0x02,
    0x18, 0xA4, 0xA4, 0x9C, 0x78,
    0x7F, 0x08, 0x04, 0x04, 0x78,
    0x00, 0x44, 0x7D, 0x40, 0x00,
    0x20, 0x40, 0x40, 0x3D, 0x00,
    0x7F, 0x10, 0x28, 0x44, 0x00,
    0x00, 0x41, 0x7F, 0x40, 0x00,
    0x7C, 0x04, 0x78, 0x04, 0x78,
    0x7C, 0x08, 0x04, 0x04, 0x78,
    0x38, 0x44, 0x44, 0x44, 0x38,
    0xFC, 0x18, 0x24, 0x24, 0x18,
    0x18, 0x24, 0x24, 0x18, 0xFC,
    0x7C, 0x08, 0x04, 0x04, 0x08,
    0x48, 0x54, 0x54, 0x54, 0x24,
    0x04, 0x04, 0x3F, 0x44, 0x24,
    0x3C, 0x40, 0x40, 0x20, 0x7C,
    0x1C, 0x20, 0x40, 0x20, 0x1C,
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    0x44, 0x28, 0x10, 0x28, 0x44,
    0x4C, 0x90, 0x90, 0x90, 0x7C,
    0x44, 0x64, 0x54, 0x4C, 0x44,
    0x00, 0x08, 0x36, 0x41, 0x00,
    0x00, 0x00, 0x77, 0x00, 0x00,
    0x00, 0x41, 0x36, 0x08, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x02,
    0x3C, 0x26, 0x23, 0x26, 0x3C,
    0x1E, 0xA1, 0xA1, 0x61, 0x12,
    0x3A, 0x40, 0x40, 0x20, 0x7A,
    0x38, 0x54, 0x54, 0x55, 0x59,
    0x21, 0x55, 0x55, 0x79, 0x41,
    0x21, 0x54, 0x54, 0x78, 0x41,
    0x21, 0x55, 0x54, 0x78, 0x40,
    0x20, 0x54, 0x55, 0x79, 0x40,
    0x0C, 0x1E, 0x52, 0x72, 0x12,
    0x39, 0x55, 0x55, 0x55, 0x59,
    0x39, 0x54, 0x54, 0x54, 0x59,
    0x39, 0x55, 0x54, 0x54, 0x58,
    0x00, 0x00, 0x45, 0x7C, 0x41,
    0x00, 0x02, 0x45, 0x7D, 0x42,
    0x00, 0x01, 0x45, 0x7C, 0x40,
    0xF0, 0x29, 0x24, 0x29, 0xF0,
    0xF0, 0x28, 0x25, 0x28, 0xF0,
    0x7C, 0x54, 0x55, 0x45, 0x00,
    0x20, 0x54, 0x54, 0x7C, 0x54,
    0x7C, 0x0A, 0x09, 0x7F, 0x49,
    0x32, 0x49, 0x49, 0x49, 0x32,
    0x32, 0x48, 0x48, 0x48, 0x32,
    0x32, 0x4A, 0x48, 0x48, 0x30,
    0x3A, 0x41, 0x41, 0x21, 0x7A,
    0x3A, 0x42, 0x40, 0x20, 0x78,
    0x00, 0x9D, 0xA0, 0xA0, 0x7D,
    0x39, 0x44, 0x44, 0x44, 0x39,
    0x3D, 0x40, 0x40, 0x40, 0x3D,
    0x3C, 0x24, 0xFF, 0x24, 0x24,
    0x48, 0x7E, 0x49, 0x43, 0x66,
    0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
    0xFF, 0x09, 0x29, 0xF6, 0x20,
    0xC0, 0x88, 0x7E, 0x09, 0x03,
    0x20, 0x54, 0x54, 0x79, 0x41,
    0x00, 0x00, 0x44, 0x7D, 0x41,
    0x30, 0x48, 0x48, 0x4A, 0x32,
    0x38, 0x40, 0x40, 0x22, 0x7A,
    0x00, 0x7A, 0x0A, 0x0A, 0x72,
    0x7D, 0x0D, 0x19, 0x31, 0x7D,
    0x26, 0x29, 0x29, 0x2F, 0x28,
    0x26, 0x29, 0x29, 0x29, 0x26,
    0x30, 0x48, 0x4D, 0x40, 0x20,
    0x38, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x38,
    0x2F, 0x10, 0xC8, 0xAC, 0xBA,
    0x2F, 0x10, 0x28, 0x34, 0xFA,
    0x00, 0x00, 0x7B, 0x00, 0x00,
    0x08, 0x14, 0x2A, 0x14, 0x22,
    0x22, 0x14, 0x2A, 0x14, 0x08,
    0xAA, 0x00, 0x55, 0x00, 0xAA,
    0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x00, 0x00, 0x00, 0xFF, 0x00,
    0x10, 0x10, 0x10, 0xFF, 0x00,
    0x14, 0x14, 0x14, 0xFF, 0x00,
    0x10, 0x10, 0xFF, 0x00, 0xFF,
    0x10, 0x10, 0xF0, 0x10, 0xF0,
    0x14, 0x14, 0x14, 0xFC, 0x00,
    0x14, 0x14, 0xF7, 0x00, 0xFF,
    0x00, 0x00, 0xFF, 0x00, 0xFF,
    0x14, 0x14, 0xF4, 0x04, 0xFC,
    0x14, 0x14, 0x17, 0x10, 0x1F,
    0x10, 0x10, 0x1F, 0x10, 0x1F,
    0x14, 0x14, 0x14, 0x1F, 0x00,
    0x10, 0x10, 0x10, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0x1F, 0x10,
    0x10, 0x10, 0x10, 0x1F, 0x10,
    0x10, 0x10, 0x10, 0xF0, 0x10,
    0x00, 0x00, 0x00, 0xFF, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0xFF, 0x10,
    0x00, 0x00, 0x00, 0xFF, 0x14,
    0x00, 0x00, 0xFF, 0x00, 0xFF,
    0x00, 0x00, 0x1F, 0x10, 0x17,
    0x00, 0x00, 0xFC, 0x04, 0xF4,
    0x14, 0x14, 0x17, 0x10, 0x17,
    0x14, 0x14, 0xF4, 0x04, 0xF4,
    0x00, 0x00, 0xFF, 0x00, 0xF7,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0xF7, 0x00, 0xF7,
    0x14, 0x14, 0x14, 0x17, 0x14,
    0x10, 0x10, 0x1F, 0x10, 0x1F,
    0x14, 0x14, 0x14, 0xF4, 0x14,
    0x10, 0x10, 0xF0, 0x10, 0xF0,
    0x00, 0x00, 0x1F, 0x10, 0x1F,
    0x00, 0x00, 0x00, 0x1F, 0x14,
    0x00, 0x00, 0x00, 0xFC, 0x14,
    0x00, 0x00, 0xF0, 0x10, 0xF0,
    0x10, 0x10, 0xFF, 0x10, 0xFF,
    0x14, 0x14, 0x14, 0xFF, 0x14,
    0x10, 0x10, 0x10, 0x1F, 0x00,
    0x00, 0x00, 0x00, 0xF0, 0x10,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0xFF, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0xFF,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x38, 0x44, 0x44, 0x38, 0x44,
    0x7C, 0x2A, 0x2A, 0x3E, 0x14,
    0x7E, 0x02, 0x02, 0x06, 0x06,
    0x02, 0x7E, 0x02, 0x7E, 0x02,
    0x63, 0x55, 0x49, 0x41, 0x63,
    0x38, 0x44, 0x44, 0x3C, 0x04,
    0x40, 0x7E, 0x20, 0x1E, 0x20,
    0x06, 0x02, 0x7E, 0x02, 0x02,
    0x99, 0xA5, 0xE7, 0xA5, 0x99,
    0x1C, 0x2A, 0x49, 0x2A, 0x1C,
    0x4C, 0x72, 0x01, 0x72, 0x4C,
    0x30, 0x4A, 0x4D, 0x4D, 0x30,
    0x30, 0x48, 0x78, 0x48, 0x30,
    0xBC, 0x62, 0x5A, 0x46, 0x3D,
    0x3E, 0x49, 0x49, 0x49, 0x00,
    0x7E, 0x01, 0x01, 0x01, 0x7E,
    0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
    0x44, 0x44, 0x5F, 0x44, 0x44,
    0x40, 0x51, 0x4A, 0x44, 0x40,
    0x40, 0x44, 0x4A, 0x51, 0x40,
    0x00, 0x00, 0xFF, 0x01, 0x03,
    0xE0, 0x80, 0xFF, 0x00, 0x00,
    0x08, 0x08, 0x6B, 0x6B, 0x08,
    0x36, 0x12, 0x36, 0x24, 0x36,
    0x06, 0x0F, 0x09, 0x0F, 0x06,
    0x00, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x00, 0x10, 0x10, 0x00,
    0x30, 0x40, 0xFF, 0x01, 0x01,
    0x00, 0x1F, 0x01, 0x01, 0x1E,
    0x00, 0x19, 0x1D, 0x17, 0x12,
    0x00, 0x3C, 0x3C, 0x3C, 0x3C,
    0x00, 0x00, 0x00, 0x00, 0x00,
};

// Display definitions
#define SSD1306_LCDWIDTH                                128
#define SSD1306_LCDHEIGHT                               64

#define SSD1306_SETCONTRAST                             0x81
#define SSD1306_DISPLAYALLON_RESUME                     0xA4
#define SSD1306_DISPLAYALLON                            0xA5
#define SSD1306_NORMALDISPLAY                           0xA6
#define SSD1306_INVERTDISPLAY                           0xA7
#define SSD1306_DISPLAYOFF                              0xAE
#define SSD1306_DISPLAYON                               0xAF

#define SSD1306_SETDISPLAYOFFSET                        0xD3
#define SSD1306_SETCOMPINS                              0xDA

#define SSD1306_SETVCOMDETECT                           0xDB

#define SSD1306_SETDISPLAYCLOCKDIV                      0xD5
#define SSD1306_SETPRECHARGE                            0xD9

#define SSD1306_SETMULTIPLEX                            0xA8

#define SSD1306_SETLOWCOLUMN                            0x00
#define SSD1306_SETHIGHCOLUMN                           0x10

#define SSD1306_SETSTARTLINE                            0x40

#define SSD1306_MEMORYMODE                              0x20
#define SSD1306_COLUMNADDR                              0x21
#define SSD1306_PAGEADDR                                0x22

#define SSD1306_COMSCANINC                              0xC0
#define SSD1306_COMSCANDEC                              0xC8

#define SSD1306_SEGREMAP                                0xA0

#define SSD1306_CHARGEPUMP                              0x8D

#define SSD1306_EXTERNALVCC                             0x01
#define SSD1306_SWITCHCAPVCC                            0x02

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL                         0x2F
#define SSD1306_DEACTIVATE_SCROLL                       0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA                0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL                 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL                  0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL    0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL     0x2A

typedef enum
{
    DISPLAY_DATA = 0,
    DISPLAY_COMMAND
}DisplayTransferType_t;

/*!
 * Holds the display context
 */
static struct Display_s
{
    int16_t Width;
    int16_t Height;
    int16_t CursorX;
    int16_t CursorY;
    uint8_t Rotation;
    uint8_t TextSize;
    DisplayColor_t TextColor;
    DisplayColor_t TextBgColor;
    bool Wrap;
    uint8_t VccState;
    uint8_t Buffer[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8];
    struct Pins_s
    {
        Gpio_t NReset;
        Gpio_t Nss;
        Gpio_t Dc;
        Gpio_t NEnable;
        Spi_t  Spi;
    }Pins;
}Display;

#ifndef SwapInt16
/*!
 * \brief Swaps a and b values
 */
#define SwapInt16( a, b ) \
{                         \
    int16_t t = a;        \
    a = b;                \
    b = t;                \
}
#endif

/*!
 * \brief Sends a data buffer to the display
 *
 * \param buffer Buffer to be sent
 * \param size   Buffer size to be sent
 * \param type   Buffer data type [DISPLAY_DATA, DISPLAY_COMMAND]
 */
static void DisplayWriteBuffer( uint8_t *buffer, uint16_t size, DisplayTransferType_t type );

/*!
 * \brief Display controller initialization
 */
static void DisplayControllerInit( void );

void DisplayInit( void )
{
    Display.Width = SSD1306_LCDWIDTH;
    Display.Height = SSD1306_LCDHEIGHT;
    Display.CursorX = 0;
    Display.CursorY = 0;
    Display.Rotation = 0;
    Display.TextSize = 1;
    Display.TextColor = DISPLAY_WHITE;
    Display.TextBgColor = DISPLAY_BLACK;
    Display.Wrap = true;

    Display.VccState = SSD1306_SWITCHCAPVCC;

    GpioInit( &Display.Pins.NReset, OLED_RST, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Display.Pins.Nss, OLED_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Display.Pins.Dc, OLED_DC, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Display.Pins.NEnable, OLED_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    SpiInit( &Display.Pins.Spi, BLE_OLED_MOSI, BLE_OLED_MISO, BLE_OLED_SCK, NC );

    DisplayReset( );
    DisplayControllerInit( );

    DisplayClear( );
    DisplayFillScreen( DISPLAY_BLACK );
    DisplaySetCursor( 3, 0 );
    DisplaySetTextSize( 1 );
    DisplaySetTextColor( DISPLAY_WHITE );
    DisplayUpdate( );
}

void DisplayReset( void )
{
    GpioWrite( &Display.Pins.NReset, 1 );

    // VDD (3.3V) goes high at start, lets just chill for a ms
    DelayMs( 1 );

    // bring reset low
    GpioWrite( &Display.Pins.NReset, 0 );

    // wait 10ms
    DelayMs( 10 );

    // bring out of reset
    GpioWrite( &Display.Pins.NReset, 1 );

    DelayMs( 1 );
}

void DisplaySendCommand( uint8_t cmd )
{
    DisplayWriteBuffer( &cmd, 1, DISPLAY_COMMAND );
}

void DisplaySendData( uint8_t *buffer, uint16_t size )
{
    DisplayWriteBuffer( buffer, size, DISPLAY_DATA );
}

static void DisplayWriteBuffer( uint8_t *buffer, uint16_t size, DisplayTransferType_t type )
{
    uint16_t i;

    GpioWrite( &Display.Pins.Nss, 1 );

    if( type == DISPLAY_COMMAND )
    {
        GpioWrite( &Display.Pins.Dc, 0 );
    }
    else if( type == DISPLAY_DATA )
    {
        GpioWrite( &Display.Pins.Dc, 1 );
    }

    //NSS = 0;
    GpioWrite( &Display.Pins.Nss, 0 );

    for( i = 0; i < size; i++ )
    {
        SpiInOut( &Display.Pins.Spi, buffer[i] );
    }
    //NSS = 1;
    GpioWrite( &Display.Pins.Nss, 1 );
}

static void DisplayControllerInit( void )
{
    // Init sequence
    DisplaySendCommand( SSD1306_DISPLAYOFF );                    // 0xAE    // blank display here
    DisplaySendCommand( SSD1306_SETDISPLAYCLOCKDIV );            // 0xD5
    DisplaySendCommand( 0x80 );                                  // the suggested ratio 0x80

    DisplaySendCommand( SSD1306_SETMULTIPLEX );                  // 0xA8
    DisplaySendCommand( SSD1306_LCDHEIGHT - 1 );

    DisplaySendCommand( SSD1306_SETDISPLAYOFFSET );              // 0xD3
    DisplaySendCommand( 0x0 );                                   // no offset
    DisplaySendCommand( SSD1306_SETSTARTLINE | 0x0 );            // line #0
    DisplaySendCommand( SSD1306_CHARGEPUMP );                    // 0x8D
    if( Display.VccState == SSD1306_EXTERNALVCC )
    {
        DisplaySendCommand( 0x10 );
    }
    else
    {
        DisplaySendCommand( 0x14 );
    }
    DisplaySendCommand( SSD1306_MEMORYMODE );                    // 0x20
    DisplaySendCommand( 0x00 );                                  // 0x0 act like ks0108
    DisplaySendCommand( SSD1306_SEGREMAP | 0x1 );
    DisplaySendCommand( SSD1306_COMSCANDEC );

    DisplaySendCommand( SSD1306_SETCOMPINS );                    // 0xDA
    DisplaySendCommand( 0x12);
    DisplaySendCommand( SSD1306_SETCONTRAST );                   // 0x81
    if( Display.VccState == SSD1306_EXTERNALVCC )
    {
        DisplaySendCommand( 0x9F );
    }
    else
    {
        DisplaySendCommand( 0xCF );
    }

    DisplaySendCommand( SSD1306_SETPRECHARGE );                  // 0xd9
    if( Display.VccState == SSD1306_EXTERNALVCC )
    {
        DisplaySendCommand( 0x22 );
    }
    else
    {
        DisplaySendCommand( 0xF1 );
    }
    DisplaySendCommand( SSD1306_SETVCOMDETECT );                 // 0xDB
    DisplaySendCommand( 0x40);
    DisplaySendCommand( SSD1306_DISPLAYALLON_RESUME );           // 0xA4
    DisplaySendCommand( SSD1306_NORMALDISPLAY );                 // 0xA6

    DisplaySendCommand( SSD1306_DEACTIVATE_SCROLL );

    DisplaySendCommand( SSD1306_DISPLAYON );                     //--turn on oled panel

    DelayMs( 100 );
}

void DisplayOn( void )
{
    DisplaySendCommand( SSD1306_DISPLAYON );                     //--turn on oled panel
}

void DisplayOff( void )
{
    DisplaySendCommand( SSD1306_DISPLAYOFF );                    //--turn off oled panel
}

void DisplayClear( void )
{
    memset1( Display.Buffer, 0, sizeof( Display.Buffer ) );
}

void DisplayInvertColors( bool invert )
{
    if( invert == true )
    {
        DisplaySendCommand( SSD1306_INVERTDISPLAY );
    }
    else
    {
        DisplaySendCommand( SSD1306_NORMALDISPLAY );
    }
}

void DisplayUpdate( void )
{
    DisplaySendCommand( SSD1306_SETLOWCOLUMN );
    DisplaySendCommand( SSD1306_SETHIGHCOLUMN );
    DisplaySendCommand( SSD1306_SETSTARTLINE );

    DisplaySendData( Display.Buffer, sizeof( Display.Buffer ) );
}

void DisplaySetCursor( int16_t x, int16_t y )
{
    Display.CursorX = x;
    Display.CursorY = y;
}

int16_t DisplayGetCursorX( void )
{
    return Display.CursorX;
}

int16_t DisplayGetCursorY( void )
{
    return Display.CursorY;
}

void DisplaySetTextSize( uint8_t s )
{
    Display.TextSize = ( s > 0 ) ? s : 1;
}

void DisplaySetTextColor( DisplayColor_t color )
{
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    Display.TextColor = Display.TextBgColor = color;
}

void DisplaySetFgAndBg( DisplayColor_t fg, DisplayColor_t bg )
{
    Display.TextColor = fg;
    Display.TextBgColor = bg;
}

void DisplaySetTextWrap( bool w )
{
    Display.Wrap = w;
}

uint8_t DisplayGetRotation( void )
{
    return Display.Rotation;
}

void DisplaySetRotation( uint8_t x )
{
    Display.Rotation = ( x & 3 );
    switch( Display.Rotation )
    {
    case 0:
    case 2:
        Display.Width = SSD1306_LCDWIDTH;
        Display.Height = SSD1306_LCDHEIGHT;
        break;
    case 1:
    case 3:
        Display.Width = SSD1306_LCDHEIGHT;
        Display.Height = SSD1306_LCDWIDTH;
        break;
    }
}

void DisplayDrawPixel( int16_t x, int16_t y, DisplayColor_t color )
{
    if( ( x >= Display.Width ) || ( y >= Display.Height ) )
    {
        return;
    }

    // check rotation, move pixel around if necessary
    switch( DisplayGetRotation( ) )
    {
        case 1:
            SwapInt16( x, y );
            x =  - x - 1;
            break;
        case 2:
            x =  - x - 1;
            y = Display.Height - y - 1;
            break;
        case 3:
            SwapInt16( x, y );
            y = Display.Height - y - 1;
            break;
    }

    // x is which column
    switch( color )
    {
        case DISPLAY_WHITE:
            Display.Buffer[x + ( y / 8 ) * SSD1306_LCDWIDTH] |=  ( 1 << ( y & 7 ) );
            break;
        case DISPLAY_BLACK:
            Display.Buffer[x + ( y / 8 ) * SSD1306_LCDWIDTH] &= ~( 1 << ( y & 7 ) );
            break;
        case DISPLAY_INVERSE:
            Display.Buffer[x + ( y / 8 ) * SSD1306_LCDWIDTH] ^=  ( 1 << ( y & 7 ) );
            break;
    }
}

void DisplayDrawLine( int16_t x0, int16_t y0, int16_t x1, int16_t y1, DisplayColor_t color )
{
    int16_t steep = abs( y1 - y0 ) > abs( x1 - x0 );

    if( steep != 0 )
    {
        SwapInt16( x0, y0 ); // entering here
        SwapInt16( x1, y1 );
    }

    if( x0 > x1 )
    {
        SwapInt16( x0, x1 ); // but not here
        SwapInt16( y0, y1 );
    }

    int16_t dx, dy;
    dx = x1 - x0;           // 63 - 0
    dy = abs( y1 - y0 );    // 0 - 0

    int16_t err = dx / 2;   //63/2
    int16_t ystep;

    if( y0 < y1 )
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for( ; x0 <= x1; x0++ ) // 0, 63
    {
        if( steep != 0 )
        {
            DisplayDrawPixel( y0, x0, color ); //0 0 1
        }
        else
        {
            DisplayDrawPixel( x0, y0, color );
        }
        err -= dy;
        if( err < 0 )
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void DisplayDrawVerticalLine( int16_t x, int16_t y, int16_t h, DisplayColor_t color )
{
    DisplayDrawLine( x, y, x, y + h - 1, color );
}

void DisplayDrawHorizontalLine( int16_t x, int16_t y, int16_t w, DisplayColor_t color )
{
    DisplayDrawLine( x, y, x + w - 1, y, color );
}

void DisplayDrawRect( int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor_t color )
{
    DisplayDrawHorizontalLine( x, y, w, color );
    DisplayDrawHorizontalLine( x, y + h - 1, w, color );
    DisplayDrawVerticalLine( x, y, h, color );
    DisplayDrawVerticalLine( x + w - 1, y, h, color );
}

void DisplayFillRect( int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor_t color )
{
    // Update in subclasses if desired!
    for( int16_t i = x; i < ( x + w ); i++ )
    {
        DisplayDrawVerticalLine( i, y, h, color );
    }
}

void DisplayFillScreen( DisplayColor_t color )
{
    DisplayFillRect( 0, 0, Display.Width, Display.Height, color );
}

// Draw a triangle
void DisplayDrawTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, DisplayColor_t color )
{
    DisplayDrawLine( x0, y0, x1, y1, color );
    DisplayDrawLine( x1, y1, x2, y2, color );
    DisplayDrawLine( x2, y2, x0, y0, color );
}


// Fill a triangle
void DisplayFillTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, DisplayColor_t color )
{
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if( y0 > y1 )
    {
        SwapInt16( y0, y1 ); SwapInt16( x0, x1 );
    }

    if( y1 > y2 )
    {
        SwapInt16( y2, y1 ); SwapInt16( x2, x1 );
    }

    if( y0 > y1 )
    {
        SwapInt16( y0, y1 ); SwapInt16( x0, x1 );
    }

    if( y0 == y2 )
    { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if( x1 < a )
        {
            a = x1;
        }
        else if( x1 > b )
        {
            b = x1;
        }
        if( x2 < a )
        {
            a = x2;
        }
        else if( x2 > b )
        {
            b = x2;
        }
        DisplayDrawHorizontalLine( a, y0, b - a + 1, color );
        return;
    }

    int16_t dx01 = x1 - x0;
    int16_t dy01 = y1 - y0;
    int16_t dx02 = x2 - x0;
    int16_t dy02 = y2 - y0;
    int16_t dx12 = x2 - x1;
    int16_t dy12 = y2 - y1;
    int32_t sa   = 0;
    int32_t sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if( y1 == y2 )
    {
        last = y1;   // Include y1 scanline
    }
    else
    {
        last = y1-1; // Skip it
    }

    for( y = y0; y <= last; y++ )
    {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

#if 0       // longhand:
        a = x0 + ( x1 - x0 ) * ( y - y0 ) / ( y1 - y0 );
        b = x0 + ( x2 - x0 ) * ( y - y0 ) / ( y2 - y0 );
#endif
        if( a > b )
        {
            SwapInt16( a, b );
        }
        DisplayDrawHorizontalLine( a, y, b - a + 1, color );
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * ( y - y1 );
    sb = dx02 * ( y - y0 );

    for( ; y <= y2; y++ )
    {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

#if 0   // longhand:
        a = x1 + ( x2 - x1 ) * ( y - y1 ) / ( y2 - y1 );
        b = x0 + ( x2 - x0 ) * ( y - y0 ) / ( y2 - y0 );
#endif
        if( a > b )
        {
            SwapInt16( a, b );
        }
        DisplayDrawHorizontalLine( a, y, b - a + 1, color );
    }
}

void DisplayDrawChar( int16_t x, int16_t y, unsigned char c, DisplayColor_t color, DisplayColor_t bg, uint8_t size )
{
    if( ( x >= Display.Width )       || // Clip right
        ( y >= Display.Height )      || // Clip bottom
        ( ( x + 6 * size - 1 ) < 0 ) || // Clip left
        ( ( y + 8 * size - 1 ) < 0 ) )  // Clip top
    {
        return;
    }

    for( int8_t i = 0; i < 6; i++ )
    {
        uint8_t line;

        if( i < 5 )
        {
            line = FontStd[( c * 5 ) + i];
        }
        else
        {
            line = 0x0;
        }

        for( int8_t j = 0; j < 8; j++, line >>= 1 )
        {
            if( ( line & 0x1 ) != 0 )
            {
                if( size == 1 )
                {
                    DisplayDrawPixel( x + i, y + j, color );
                }
                else
                {
                    DisplayFillRect( x + ( i * size ), y + ( j * size ), size, size, color );
                }
            }
            else
            {
                if( bg != color )
                {
                    if( size == 1 )
                    {
                        DisplayDrawPixel( x + i, y + j, bg );
                    }
                    else
                    {
                        DisplayFillRect( x + ( i * size ), y + ( j * size ), size, size, bg );
                    }
                }
            }
        }
    }
}

void DisplayPutc( uint8_t c )
{
    if( c == '\n' )
    {
        Display.CursorY += Display.TextSize * 8;
        Display.CursorX  = 0;
    }
    else if( c == '\r' )
    {
        // skip em
    }
    else
    {
        DisplayDrawChar( Display.CursorX, Display.CursorY, c, Display.TextColor, Display.TextBgColor, Display.TextSize );
        Display.CursorX += Display.TextSize * 6;

        if( ( Display.Wrap == true ) &&
            ( ( Display.CursorX + Display.TextSize * 6 ) >= Display.Width ) ) // Heading off edge?
        {
            Display.CursorX  = 0;                    // Reset x to zero
            Display.CursorY += Display.TextSize * 8; // Advance y one line
        }
    }
}

void DisplaySetLine( uint8_t line )
{
    switch( line )
    {
    default:
    case 1:
        DisplaySetCursor( 4, 14 );
        DisplaySetTextSize( 1 );
        DisplayFillRect( 5, 14, 124, 8, DISPLAY_BLACK );
        break;
    case 2:
        DisplaySetCursor( 0, 24 );
        DisplaySetTextSize( 1 );
        DisplayFillRect( 0, 24, 128, 8, DISPLAY_BLACK );
        break;
    case 3:
        DisplaySetCursor( 0, 32 );
        DisplaySetTextSize( 1 );
        DisplayFillRect( 0, 32, 128, 8, DISPLAY_BLACK );
        break;
    case 4:
        DisplaySetCursor( 0, 40 );
        DisplaySetTextSize( 1 );
        DisplayFillRect( 0, 40, 128, 8, DISPLAY_BLACK );
        break;
    case 5:
        DisplaySetCursor( 0, 48 );
        DisplaySetTextSize( 1 );
        DisplayFillRect( 0, 48, 128, 8, DISPLAY_BLACK );
        break;
    case 6:
        DisplaySetCursor( 0, 56 );
        DisplaySetTextSize( 1 );
        DisplayFillRect( 0, 56, 128, 8, DISPLAY_BLACK );
        break;
    }
}

void DisplayPrint( const char *string )
{
    for( const char *p = string; *p; p++ )
    {
        DisplayPutc( *p );
    }
}

void DisplayPrintf( const char *format, ... )
{
    va_list ap;
    char tmpStr[80];

    va_start( ap, format );

    vsnprintf( tmpStr, sizeof( tmpStr ), format, ap );

    for( char *p = &tmpStr[0]; *p; p++ )
    {
        DisplayPutc( *p );
    }

    va_end( ap );
}

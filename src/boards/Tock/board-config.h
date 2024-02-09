/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 *
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define RADIO_BUSY  1
#define RADIO_DIO_1  2
#define RADIO_DIO_3  3
#define RADIO_RESET  4
#define RADIO_NSS   0

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      1


#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__

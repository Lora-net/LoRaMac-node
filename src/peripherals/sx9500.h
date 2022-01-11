/*!
 * \file  sx9500.h
 *
 * \brief SX9500 proximity sensor driver implementation
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
#ifndef __SX9500_H__
#define __SX9500_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define SX9500_I2C_ADDRESS                          0x28

#define SX9500_REG_IRQSRC                           0x00
#define SX9500_REG_STAT                             0x01
#define SX9500_REG_IRQMSK                           0x03
#define SX9500_REG_PROXCTRL0                        0x06
#define SX9500_REG_PROXCTRL1                        0x07
#define SX9500_REG_PROXCTRL2                        0x08
#define SX9500_REG_PROXCTRL3                        0x09
#define SX9500_REG_PROXCTRL4                        0x0A
#define SX9500_REG_PROXCTRL5                        0x0B
#define SX9500_REG_PROXCTRL6                        0x0C
#define SX9500_REG_PROXCTRL7                        0x0D
#define SX9500_REG_PROXCTRL8                        0x0E
#define SX9500_REG_SENSORSEL                        0x20
#define SX9500_REG_USEMSB                           0x21
#define SX9500_REG_USELSB                           0x22
#define SX9500_REG_AVGMSB                           0x23
#define SX9500_REG_AVGLSB                           0x24
#define SX9500_REG_DIFFMSB                          0x25
#define SX9500_REG_DIFFLSB                          0x26
#define SX9500_REG_OFFSETMSB                        0x27
#define SX9500_REG_OFFSETLSB                        0x28
#define SX9500_REG_RESET                            0x7F

#define SX9500_RESET_CMD                            0xDE

LmnStatus_t SX9500Init( void );

/*!
 * \brief Resets the device
 *
 * \retval status [LMN_STATUS_OK, LMN_STATUS_ERROR]
 */
LmnStatus_t SX9500Reset( void );

/*!
 * \brief Writes a byte at specified address in the device
 *
 * \param [IN]: addr
 * \param [IN]: data
 *
 * \retval status [LMN_STATUS_OK, LMN_STATUS_ERROR]
 */
LmnStatus_t SX9500Write( uint8_t addr, uint8_t data );

/*!
 * \brief Writes a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [IN]: data
 * \param [IN]: size
 *
 * \retval status [LMN_STATUS_OK, LMN_STATUS_ERROR]
 */
LmnStatus_t SX9500WriteBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Reads a byte at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 *
 * \retval status [LMN_STATUS_OK, LMN_STATUS_ERROR]
 */
LmnStatus_t SX9500Read( uint8_t addr, uint8_t *data );

/*!
 * \brief Reads a buffer at specified address in the device
 *
 * \param [IN]: addr
 * \param [OUT]: data
 * \param [IN]: size
 *
 * \retval status [LMN_STATUS_OK, LMN_STATUS_ERROR]
 */
LmnStatus_t SX9500ReadBuffer( uint8_t addr, uint8_t *data, uint8_t size );

/*!
 * \brief Sets the I2C device slave address
 *
 * \param [IN]: addr
 */
void SX9500SetDeviceAddr( uint8_t addr );

/*!
 * \brief Gets the I2C device slave address
 *
 * \retval: addr Current device slave address
 */
uint8_t SX9500GetDeviceAddr( void );

/*!
 * \brief Goes into a loop until a successful capacitive proximity detection
 */
void SX9500LockUntilDetection( void );

#ifdef __cplusplus
}
#endif

#endif // __SX1509_H__

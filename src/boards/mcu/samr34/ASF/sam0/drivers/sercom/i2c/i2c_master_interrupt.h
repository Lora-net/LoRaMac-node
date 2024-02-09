/**
 * \file
 *
 * \brief SAM SERCOM I2C Master Interrupt Driver
 *
 * Copyright (c) 2012-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef I2C_MASTER_INTERRUPT_H_INCLUDED
#define I2C_MASTER_INTERRUPT_H_INCLUDED

#include "i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup asfdoc_sam0_sercom_i2c_group
 * @{
 *
 */

/**
 * \name Callbacks
 * @{
 */
#if !defined(__DOXYGEN__)
void _i2c_master_interrupt_handler(
		uint8_t instance);
#endif

void i2c_master_register_callback(
		struct i2c_master_module *const module,
		i2c_master_callback_t callback,
		enum i2c_master_callback callback_type);

void i2c_master_unregister_callback(
		struct i2c_master_module *const module,
		enum i2c_master_callback callback_type);

/**
 * \brief Enables callback
 *
 * Enables the callback specified by the callback_type.
 *
 * \param[in,out] module         Pointer to the software module struct
 * \param[in]     callback_type  Callback type to enable
 */
static inline void i2c_master_enable_callback(
		struct i2c_master_module *const module,
		enum i2c_master_callback callback_type)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	/* Mark callback as enabled */
	module->enabled_callback |= (1 << callback_type);
}

/**
 * \brief Disables callback
 *
 * Disables the callback specified by the callback_type.
 *
 * \param[in,out] module         Pointer to the software module struct
 * \param[in]     callback_type  Callback type to disable
 */
static inline void i2c_master_disable_callback(
		struct i2c_master_module *const module,
		enum i2c_master_callback callback_type)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	/* Mark callback as disabled */
	module->enabled_callback &= ~(1 << callback_type);
}

/** @} */

/**
 * \name Read and Write, Interrupt-driven
 * @{
 */

enum status_code i2c_master_read_bytes(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);
 
enum status_code i2c_master_read_packet_job(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);

enum status_code i2c_master_read_packet_job_no_stop(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);

enum status_code i2c_master_read_packet_job_no_nack(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);

enum status_code i2c_master_write_bytes(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);

enum status_code i2c_master_write_packet_job(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);

enum status_code i2c_master_write_packet_job_no_stop(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet);

/**
 * \brief Cancel any currently ongoing operation
 *
 * Terminates the running transfer operation.
 *
 * \param[in,out] module  Pointer to software module structure
 */
static inline void i2c_master_cancel_job(
		struct i2c_master_module *const module)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	/* Set buffer to 0 */
	module->buffer_remaining = 0;
	/* Update status */
	module->status = STATUS_ABORTED;
}

/**
 * \brief Get status from ongoing job
 *
 * Will return the status of a transfer operation.
 *
 * \param[in] module  Pointer to software module structure
 *
 * \return Last status code from transfer operation.
 * \retval STATUS_OK                    No error has occurred
 * \retval STATUS_BUSY                  If transfer is in progress
 * \retval STATUS_BUSY                  If master module is busy
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 * \retval STATUS_ERR_TIMEOUT           If timeout occurred
 * \retval STATUS_ERR_OVERFLOW          If slave did not acknowledge last sent
 *                                      data, indicating that slave does not
 *                                      want more data and was not able to read
 */
static inline enum status_code i2c_master_get_job_status(
		struct i2c_master_module *const module)
{
	/* Check sanity */
	Assert(module);
	Assert(module->hw);

	/* Return current status code */
	return module->status;
}

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* I2C_MASTER_INTERRUPT_H_INCLUDED */

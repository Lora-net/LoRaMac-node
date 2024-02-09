/**
 * \file
 *
 * \brief SAM I2C Master Interrupt Driver
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

#include "i2c_master_interrupt.h"

extern enum status_code _i2c_master_wait_for_bus(
		struct i2c_master_module *const module);

extern enum status_code _i2c_master_address_response(
		struct i2c_master_module *const module);

extern enum status_code _i2c_master_send_hs_master_code(
		struct i2c_master_module *const module,
		uint8_t hs_master_code);;

/**
 * \internal
 * Read next data. Used by interrupt handler to get next data byte from slave.
 *
 * \param[in,out] module  Pointer to software module structure
 */
static void _i2c_master_read(
		struct i2c_master_module *const module)
{
	/* Sanity check arguments. */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);
	bool sclsm_flag = i2c_module->CTRLA.bit.SCLSM;

	/* Find index to save next value in buffer */
	uint16_t buffer_index = module->buffer_length;
	buffer_index -= module->buffer_remaining;

	module->buffer_remaining--;

	if (sclsm_flag) {
		if (module->send_nack && module->buffer_remaining == 1) {
			/* Set action to NACK. */
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
		}
	} else {
		if (module->send_nack && module->buffer_remaining == 0) {
			/* Set action to NACK. */
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
		}
	}

	if (module->buffer_remaining == 0) {
		if (module->send_stop) {
			/* Send stop condition */
			_i2c_master_wait_for_sync(module);
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		}
	}
	
	/* Read byte from slave and put in buffer */
	_i2c_master_wait_for_sync(module);
	module->buffer[buffer_index] = i2c_module->DATA.reg;
}

/**
 * \internal
 *
 * Write next data. Used by interrupt handler to send next data byte to slave.
 *
 * \param[in,out] module  Pointer to software module structure
 */
static void _i2c_master_write(struct i2c_master_module *const module)
{
	/* Sanity check arguments. */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Check for ack from slave */
	if (i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
	{
		/* Set status */
		module->status = STATUS_ERR_OVERFLOW;
		/* Do not write more data */
		return;
	}

	/* Find index to get next byte in buffer */
	uint16_t buffer_index = module->buffer_length;
	buffer_index -= module->buffer_remaining;

	module->buffer_remaining--;

	/* Write byte from buffer to slave */
	_i2c_master_wait_for_sync(module);
	i2c_module->DATA.reg = module->buffer[buffer_index];
}

/**
 * \internal
 * Acts on slave address response. Checks for errors concerning master->slave
 * handshake.
 *
 * \param[in,out] module  Pointer to software module structure
 */
static void _i2c_master_async_address_response(
		struct i2c_master_module *const module)
{
	/* Sanity check arguments. */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Check for error. Ignore bus-error; workaround for bus state stuck in
	 * BUSY.
	 */
	if (i2c_module->INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB)
	{
		/* Clear write interrupt flag */
		i2c_module->INTFLAG.reg = SERCOM_I2CM_INTENCLR_MB;

		/* Check arbitration */
		if (i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_ARBLOST) {
			/* Return busy */
			module->status = STATUS_ERR_PACKET_COLLISION;
		}
		/* No slave responds */
		else if (i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_RXNACK) {
			module->status           = STATUS_ERR_BAD_ADDRESS;
			module->buffer_remaining = 0;

			if (module->send_stop) {
				/* Send stop condition */
				_i2c_master_wait_for_sync(module);
				i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
			}
		}
	}

	module->buffer_length = module->buffer_remaining;

	/* Check for status OK. */
	if (module->status == STATUS_BUSY) {
		/* Call function based on transfer direction. */
		if (module->transfer_direction == I2C_TRANSFER_WRITE) {
			_i2c_master_write(module);
		} else {
			_i2c_master_read(module);
		}
	}
}

/**
 * \brief Registers callback for the specified callback type
 *
 * Associates the given callback function with the
 * specified callback type.
 *
 * To enable the callback, the \ref i2c_master_enable_callback function
 * must be used.
 *
 * \param[in,out]  module         Pointer to the software module struct
 * \param[in]      callback       Pointer to the function desired for the
 *                                specified callback
 * \param[in]      callback_type  Callback type to register
 */
void i2c_master_register_callback(
		struct i2c_master_module *const module,
		const i2c_master_callback_t callback,
		enum i2c_master_callback callback_type)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(callback);

	/* Register callback */
	module->callbacks[callback_type] = callback;

	/* Set corresponding bit to set callback as registered */
	module->registered_callback |= (1 << callback_type);
}

/**
 * \brief Unregisters callback for the specified callback type
 *
 * When called, the currently registered callback for the given callback type
 * will be removed.
 *
 * \param[in,out] module         Pointer to the software module struct
 * \param[in]     callback_type  Specifies the callback type to unregister
 */
void i2c_master_unregister_callback(
		struct i2c_master_module *const module,
		enum i2c_master_callback callback_type)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	/* Register callback */
	module->callbacks[callback_type] = NULL;

	/* Clear corresponding bit to set callback as unregistered */
	module->registered_callback &= ~(1 << callback_type);
}

/**
 * \internal
 * Starts a read bytes operation.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting reading I<SUP>2</SUP>C packet.
 * \retval STATUS_OK    If reading was started successfully
 * \retval STATUS_BUSY  If module is currently busy with another transfer
 */
enum status_code i2c_master_read_bytes(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Save packet to software module */
	module->buffer             = packet->data;
	module->buffer_remaining   = packet->data_length;
	module->transfer_direction = I2C_TRANSFER_READ;
	module->status             = STATUS_BUSY;
	module->send_stop = false;
	module->send_nack = false;
	
	/* Enable interrupts */
	i2c_module->INTENSET.reg =
			SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

	return STATUS_OK;
}

/**
 * \internal
 * Starts a read packet operation.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting reading I<SUP>2</SUP>C packet.
 * \retval STATUS_OK    If reading was started successfully
 * \retval STATUS_BUSY  If module is currently busy with another transfer
 */
static enum status_code _i2c_master_read_packet(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);
	enum status_code tmp_status;

	/* Save packet to software module */
	module->buffer             = packet->data;
	module->buffer_remaining   = packet->data_length;
	module->transfer_direction = I2C_TRANSFER_READ;
	module->status             = STATUS_BUSY;

	bool sclsm_flag = i2c_module->CTRLA.bit.SCLSM;

	/* Switch to high speed mode */
	if (packet->high_speed) {
		_i2c_master_send_hs_master_code(module, packet->hs_master_code);
	}

	/* Set action to ACK or NACK. */
	if ((sclsm_flag) && (packet->data_length == 1)) {
		i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	} else {
		i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	}

	if (packet->ten_bit_address) {
		/*
		 * Write ADDR.ADDR[10:1] with the 10-bit address. ADDR.TENBITEN must
		 * be set and read/write bit (ADDR.ADDR[0]) equal to 0.
		 */
		i2c_module->ADDR.reg = (packet->address << 1) |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos) |
			SERCOM_I2CM_ADDR_TENBITEN;

		/* Wait for response on bus. */
		tmp_status = _i2c_master_wait_for_bus(module);

		/* Set action to ack. */
		i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

		/* Check for address response error unless previous error is
		 * detected. */
		if (tmp_status == STATUS_OK) {
			tmp_status = _i2c_master_address_response(module);
		}

		if (tmp_status == STATUS_OK) {
			/* Enable interrupts */
			i2c_module->INTENSET.reg =
				SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

			/*
			 * Write ADDR[7:0] register to "11110 address[9:8] 1"
			 * ADDR.TENBITEN must be cleared
			 */
			i2c_module->ADDR.reg = (((packet->address >> 8) | 0x78) << 1) |
				(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos) |
				I2C_TRANSFER_READ;
		} else {
			return tmp_status;
		}
	} else {
		/* Enable interrupts */
		i2c_module->INTENSET.reg =
			SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

		/* Set address and direction bit. Will send start command on bus */
		i2c_module->ADDR.reg = (packet->address << 1) | I2C_TRANSFER_READ |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos);
	}

	return STATUS_OK;
}

/**
 * \brief Initiates a read packet operation
 *
 * Reads a data packet from the specified slave address on the I<SUP>2</SUP>C
 * bus. This is the non-blocking equivalent of \ref i2c_master_read_packet_wait.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting reading I<SUP>2</SUP>C packet.
 * \retval STATUS_OK    If reading was started successfully
 * \retval STATUS_BUSY  If module is currently busy with another transfer
 */
enum status_code i2c_master_read_packet_job(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

	/* Check if the I2C module is busy with a job */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}

	/* Make sure we send STOP */
	module->send_stop = true;
	module->send_nack = true;
	/* Start reading */
	return _i2c_master_read_packet(module, packet);
}

/**
 * \brief Initiates a read packet operation without sending a STOP condition when done
 *
 * Reads a data packet from the specified slave address on the I<SUP>2</SUP>C bus without
 * sending a stop condition, thus retaining ownership of the bus when done.
 * To end the transaction, a \ref i2c_master_read_packet_wait "read" or
 * \ref i2c_master_write_packet_wait "write" with stop condition must be
 * performed.
 *
 * This is the non-blocking equivalent of \ref i2c_master_read_packet_wait_no_stop.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting reading I<SUP>2</SUP>C packet.
 * \retval STATUS_OK   If reading was started successfully
 * \retval STATUS_BUSY If module is currently busy with another operation
 */
enum status_code i2c_master_read_packet_job_no_stop(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

	/* Check if the I2C module is busy with a job */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}

	/* Make sure we don't send STOP */
	module->send_stop = false;
	module->send_nack = true;
	/* Start reading */
	return _i2c_master_read_packet(module, packet);
}

/**
 * \brief Initiates a read packet operation without sending a NACK signal and a 
 * STOP condition when done
 *
 * Reads a data packet from the specified slave address on the I<SUP>2</SUP>C bus without
 * sending a nack and a stop condition, thus retaining ownership of the bus when done.
 * To end the transaction, a \ref i2c_master_read_packet_wait "read" or
 * \ref i2c_master_write_packet_wait "write" with stop condition must be
 * performed.
 *
 * This is the non-blocking equivalent of \ref i2c_master_read_packet_wait_no_stop.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting reading I<SUP>2</SUP>C packet.
 * \retval STATUS_OK   If reading was started successfully
 * \retval STATUS_BUSY If module is currently busy with another operation
 */
enum status_code i2c_master_read_packet_job_no_nack(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

	/* Check if the I2C module is busy with a job */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}

	/* Make sure we don't send STOP */
	module->send_stop = false;
	module->send_nack = false;
	/* Start reading */
	return _i2c_master_read_packet(module, packet);
}

/**
 * \internal
 * Starts a write bytes operation.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting write I<SUP>2</SUP>C bytes.
 * \retval STATUS_OK    If writing was started successfully
 * \retval STATUS_BUSY  If module is currently busy with another transfer
 */
enum status_code i2c_master_write_bytes(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Save packet to software module */
	module->buffer             = packet->data;
	module->buffer_remaining   = packet->data_length;
	module->transfer_direction = I2C_TRANSFER_WRITE;
	module->status             = STATUS_BUSY;
	module->send_stop = false;
	module->send_nack = false;
	
	/* Enable interrupts */
	i2c_module->INTENSET.reg =
			SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

	return STATUS_OK;
}

/**
 * \internal Initiates a write packet operation
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting writing I<SUP>2</SUP>C packet job.
 * \retval STATUS_OK   If writing was started successfully
 * \retval STATUS_BUSY If module is currently busy with another transfer
 */
static enum status_code _i2c_master_write_packet(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Switch to high speed mode */
	if (packet->high_speed) {
		_i2c_master_send_hs_master_code(module, packet->hs_master_code);
	}

	/* Set action to ACK. */
	i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

	/* Save packet to software module */
	module->buffer             = packet->data;
	module->buffer_remaining   = packet->data_length;
	module->transfer_direction = I2C_TRANSFER_WRITE;
	module->status             = STATUS_BUSY;

	/* Enable interrupts */
	i2c_module->INTENSET.reg =
			SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

	/* Set address and direction bit, will send start command on bus */
	if (packet->ten_bit_address) {
		i2c_module->ADDR.reg = (packet->address << 1) | I2C_TRANSFER_WRITE |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos) |
			SERCOM_I2CM_ADDR_TENBITEN;
	} else {
		i2c_module->ADDR.reg = (packet->address << 1) | I2C_TRANSFER_WRITE |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos);
	}

	return STATUS_OK;
}

/**
 * \brief Initiates a write packet operation
 *
 * Writes a data packet to the specified slave address on the I<SUP>2</SUP>C
 * bus. This is the non-blocking equivalent of \ref i2c_master_write_packet_wait.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting writing I<SUP>2</SUP>C packet job.
 * \retval STATUS_OK    If writing was started successfully
 * \retval STATUS_BUSY  If module is currently busy with another transfer
 */
enum status_code i2c_master_write_packet_job(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

	/* Check if the I2C module is busy with another job. */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}

	/* Make sure we send STOP at end*/
	module->send_stop = true;
	module->send_nack = true;
	/* Start write operation */
	return _i2c_master_write_packet(module, packet);
}

/**
 * \brief Initiates a write packet operation without sending a STOP condition when done
 *
 * Writes a data packet to the specified slave address on the I<SUP>2</SUP>C bus
 * without sending a stop condition, thus retaining ownership of the bus when
 * done. To end the transaction, a \ref i2c_master_read_packet_wait "read" or
 * \ref i2c_master_write_packet_wait "write" with stop condition or sending
 * a stop with the \ref i2c_master_send_stop function must be performed.
 *
 * This is the non-blocking equivalent of \ref i2c_master_write_packet_wait_no_stop.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of starting writing I<SUP>2</SUP>C packet job.
 * \retval STATUS_OK    If writing was started successfully
 * \retval STATUS_BUSY  If module is currently busy with another
 */
enum status_code i2c_master_write_packet_job_no_stop(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

	/* Check if the I2C module is busy with another job. */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}

	/* Do not send stop condition when done */
	module->send_stop = false;
	module->send_nack = true;
	/* Start write operation */
	return _i2c_master_write_packet(module, packet);
}

/**
 * \internal
 * Interrupt handler for I<SUP>2</SUP>C master.
 *
 * \param[in] instance  SERCOM instance that triggered the interrupt
 */
void _i2c_master_interrupt_handler(
		uint8_t instance)
{
	/* Get software module for callback handling */
	struct i2c_master_module *module =
			(struct i2c_master_module*)_sercom_instances[instance];

	Assert(module);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);
	bool sclsm_flag = i2c_module->CTRLA.bit.SCLSM;

	/* Combine callback registered and enabled masks */
	uint8_t callback_mask = module->enabled_callback;
	callback_mask &= module->registered_callback;

	/* Check if the module should respond to address ack */
	if ((module->buffer_length <= 0) && (module->buffer_remaining > 0)) {
		/* Call function for address response */
		_i2c_master_async_address_response(module);

	/* Check if buffer write is done */
	} else if ((module->buffer_length > 0) && (module->buffer_remaining <= 0) &&
			(module->status == STATUS_BUSY) &&
			(module->transfer_direction == I2C_TRANSFER_WRITE)) {
		/* Stop packet operation */
		i2c_module->INTENCLR.reg =
				SERCOM_I2CM_INTENCLR_MB | SERCOM_I2CM_INTENCLR_SB;

		module->buffer_length = 0;
		module->status        = STATUS_OK;

		if (module->send_stop) {
			/* Send stop condition */
			_i2c_master_wait_for_sync(module);
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		} else {
			/* Clear write interrupt flag */
			i2c_module->INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB;
		}
		
		if (callback_mask & (1 << I2C_MASTER_CALLBACK_WRITE_COMPLETE)) {
			module->callbacks[I2C_MASTER_CALLBACK_WRITE_COMPLETE](module);
		}

	/* Continue buffer write/read */
	} else if ((module->buffer_length > 0) && (module->buffer_remaining > 0)){
		/* Check that bus ownership is not lost */
		if ((!(i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(2))) &&
				(!(sclsm_flag && (module->buffer_remaining == 1))))	{
			module->status = STATUS_ERR_PACKET_COLLISION;
		} else if (module->transfer_direction == I2C_TRANSFER_WRITE) {
			_i2c_master_write(module);
		} else {
			_i2c_master_read(module);
		}
	}

	/* Check if read buffer transfer is complete */
	if ((module->buffer_length > 0) && (module->buffer_remaining <= 0) &&
			(module->status == STATUS_BUSY) &&
			(module->transfer_direction == I2C_TRANSFER_READ)) {
		
		/* Clear read interrupt flag */
		if (i2c_module->INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB) {
			i2c_module->INTFLAG.reg = SERCOM_I2CM_INTFLAG_SB;
		}
		/* Stop packet operation */
		i2c_module->INTENCLR.reg =
				SERCOM_I2CM_INTENCLR_MB | SERCOM_I2CM_INTENCLR_SB;
		module->buffer_length = 0;
		module->status        = STATUS_OK;

		/* Call appropriate callback if enabled and registered */
		if ((callback_mask & (1 << I2C_MASTER_CALLBACK_READ_COMPLETE))
				&& (module->transfer_direction == I2C_TRANSFER_READ)) {
			module->callbacks[I2C_MASTER_CALLBACK_READ_COMPLETE](module);
		} else if ((callback_mask & (1 << I2C_MASTER_CALLBACK_WRITE_COMPLETE))
				&& (module->transfer_direction == I2C_TRANSFER_WRITE)) {
			module->callbacks[I2C_MASTER_CALLBACK_WRITE_COMPLETE](module);
		}
	}

	/* Check for error */
	if ((module->status != STATUS_BUSY) && (module->status != STATUS_OK)) {
		/* Stop packet operation */
		i2c_module->INTENCLR.reg = SERCOM_I2CM_INTENCLR_MB |
				SERCOM_I2CM_INTENCLR_SB;

		module->buffer_length = 0;
		module->buffer_remaining = 0;

		/* Send nack and stop command unless arbitration is lost */
		if ((module->status != STATUS_ERR_PACKET_COLLISION) &&
				module->send_stop) {
			_i2c_master_wait_for_sync(module);
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT |
					SERCOM_I2CM_CTRLB_CMD(3);
		}

		/* Call error callback if enabled and registered */
		if (callback_mask & (1 << I2C_MASTER_CALLBACK_ERROR)) {
			module->callbacks[I2C_MASTER_CALLBACK_ERROR](module);
		}
	}
}

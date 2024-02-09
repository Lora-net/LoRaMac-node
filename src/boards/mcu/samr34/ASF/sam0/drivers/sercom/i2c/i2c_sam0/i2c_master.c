/**
 * \file
 *
 * \brief SAM I2C Master Driver
 *
 * Copyright (c) 2012-2020 Microchip Technology Inc. and its subsidiaries.
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

#include "i2c_master.h"

#if I2C_MASTER_CALLBACK_MODE == true
# include "i2c_master_interrupt.h"
#endif

/* Forward declaration */
enum status_code _i2c_master_wait_for_bus(
		struct i2c_master_module *const module);

enum status_code _i2c_master_address_response(
		struct i2c_master_module *const module);

enum status_code _i2c_master_send_hs_master_code(
		struct i2c_master_module *const module,
		uint8_t hs_master_code);

#if !defined(__DOXYGEN__)

/**
 * \internal Sets configurations to module
 *
 * \param[out] module  Pointer to software module structure
 * \param[in]  config  Configuration structure with configurations to set
 *
 * \return Status of setting configuration.
 * \retval STATUS_OK                        If module was configured correctly
 * \retval STATUS_ERR_ALREADY_INITIALIZED   If setting other GCLK generator than
 *                                          previously set
 * \retval STATUS_ERR_BAUDRATE_UNAVAILABLE  If given baudrate is not compatible
 *                                          with set GCLK frequency
 */
static enum status_code _i2c_master_set_config(
		struct i2c_master_module *const module,
		const struct i2c_master_config *const config)
{
	/* Sanity check arguments. */
	Assert(module);
	Assert(module->hw);
	Assert(config);

	/* Temporary variables. */
	uint32_t tmp_ctrla;
	int32_t tmp_baud = 0;
	int32_t tmp_baud_hs = 0;
	int32_t tmp_baudlow_hs = 0;
	enum status_code tmp_status_code = STATUS_OK;

	SercomI2cm *const i2c_module = &(module->hw->I2CM);
	Sercom *const sercom_hw = module->hw;

	uint8_t sercom_index = _sercom_get_sercom_inst_index(sercom_hw);

	/* Pin configuration */
	struct system_pinmux_config pin_conf;
	system_pinmux_get_config_defaults(&pin_conf);

	uint32_t pad0 = config->pinmux_pad0;
	uint32_t pad1 = config->pinmux_pad1;

	/* SERCOM PAD0 - SDA */
	if (pad0 == PINMUX_DEFAULT) {
		pad0 = _sercom_get_default_pad(sercom_hw, 0);
	}
	pin_conf.mux_position = pad0 & 0xFFFF;
	pin_conf.direction    = SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK;
	system_pinmux_pin_set_config(pad0 >> 16, &pin_conf);

	/* SERCOM PAD1 - SCL */
	if (pad1 == PINMUX_DEFAULT) {
		pad1 = _sercom_get_default_pad(sercom_hw, 1);
	}
	pin_conf.mux_position = pad1 & 0xFFFF;
	pin_conf.direction    = SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK;
	system_pinmux_pin_set_config(pad1 >> 16, &pin_conf);

	/* Save timeout on unknown bus state in software module. */
	module->unknown_bus_state_timeout = config->unknown_bus_state_timeout;

	/* Save timeout on buffer write. */
	module->buffer_timeout = config->buffer_timeout;

	/* Set whether module should run in standby. */
	if (config->run_in_standby || system_is_debugger_present()) {
		tmp_ctrla = SERCOM_I2CM_CTRLA_RUNSTDBY;
	} else {
		tmp_ctrla = 0;
	}

	/* Check and set start data hold timeout. */
	if (config->start_hold_time != I2C_MASTER_START_HOLD_TIME_DISABLED) {
		tmp_ctrla |= config->start_hold_time;
	}

	/* Check and set transfer speed */
	tmp_ctrla |= config->transfer_speed;

	/* Check and set SCL low timeout. */
	if (config->scl_low_timeout) {
		tmp_ctrla |= SERCOM_I2CM_CTRLA_LOWTOUTEN;
	}

	/* Check and set inactive bus timeout. */
	if (config->inactive_timeout != I2C_MASTER_INACTIVE_TIMEOUT_DISABLED) {
		tmp_ctrla |= config->inactive_timeout;
	}

	/* Check and set SCL clock stretch mode. */
	if (config->scl_stretch_only_after_ack_bit || (config->transfer_speed == I2C_MASTER_SPEED_HIGH_SPEED)) {
		tmp_ctrla |= SERCOM_I2CM_CTRLA_SCLSM;
	}

	/* Check and set slave SCL low extend timeout. */
	if (config->slave_scl_low_extend_timeout) {
		tmp_ctrla |= SERCOM_I2CM_CTRLA_SEXTTOEN;
	}

	/* Check and set master SCL low extend timeout. */
	if (config->master_scl_low_extend_timeout) {
		tmp_ctrla |= SERCOM_I2CM_CTRLA_MEXTTOEN;
	}

	/* Write config to register CTRLA. */
	i2c_module->CTRLA.reg |= tmp_ctrla;

	/* Set configurations in CTRLB. */
	i2c_module->CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;

	/* Find and set baudrate, considering sda/scl rise time */
	uint32_t fgclk       = system_gclk_chan_get_hz(SERCOM0_GCLK_ID_CORE + sercom_index);
	uint32_t fscl        = 1000 * config->baud_rate;
	uint32_t fscl_hs     = 1000 * config->baud_rate_high_speed;
	uint32_t trise       = config->sda_scl_rise_time_ns;
	
	tmp_baud = (int32_t)(div_ceil(
			fgclk - fscl * (10 + (fgclk * 0.000000001)* trise), 2 * fscl));
	
	/* For High speed mode, set the SCL ratio of high:low to 1:2. */
	if (config->transfer_speed == I2C_MASTER_SPEED_HIGH_SPEED) {
		tmp_baudlow_hs = (int32_t)((fgclk * 2.0) / (3.0 * fscl_hs) - 1);
		if (tmp_baudlow_hs) {
			tmp_baud_hs = (int32_t)(fgclk / fscl_hs) - 2 - tmp_baudlow_hs;
		} else {
			tmp_baud_hs = (int32_t)(div_ceil(fgclk, 2 * fscl_hs)) - 1;
		}
	}

	/* Check that baudrate is supported at current speed. */
	if (tmp_baud > 255 || tmp_baud < 0 || tmp_baud_hs > 255 || tmp_baud_hs < 0) {
		/* Baud rate not supported. */
		tmp_status_code = STATUS_ERR_BAUDRATE_UNAVAILABLE;
	}
	if (tmp_status_code != STATUS_ERR_BAUDRATE_UNAVAILABLE) {
		/* Baud rate acceptable. */
		i2c_module->BAUD.reg = SERCOM_I2CM_BAUD_BAUD(tmp_baud) |
			SERCOM_I2CM_BAUD_HSBAUD(tmp_baud_hs) | SERCOM_I2CM_BAUD_HSBAUDLOW(tmp_baudlow_hs);
	}

	return tmp_status_code;
}
#endif /* __DOXYGEN__ */

/**
 * \brief Initializes the requested I<SUP>2</SUP>C hardware module
 *
 * Initializes the SERCOM I<SUP>2</SUP>C master device requested and sets the provided
 * software module struct. Run this function before any further use of
 * the driver.
 *
 * \param[out] module  Pointer to software module struct
 * \param[in]  hw      Pointer to the hardware instance
 * \param[in]  config  Pointer to the configuration struct
 *
 * \return Status of initialization.
 * \retval STATUS_OK                        Module initiated correctly
 * \retval STATUS_ERR_DENIED                If module is enabled
 * \retval STATUS_BUSY                      If module is busy resetting
 * \retval STATUS_ERR_ALREADY_INITIALIZED   If setting other GCLK generator than
 *                                          previously set
 * \retval STATUS_ERR_BAUDRATE_UNAVAILABLE  If given baudrate is not compatible
 *                                          with set GCLK frequency
 *
 */
enum status_code i2c_master_init(
		struct i2c_master_module *const module,
		Sercom *const hw,
		const struct i2c_master_config *const config)
{
	/* Sanity check arguments. */
	Assert(module);
	Assert(hw);
	Assert(config);

	/* Initialize software module */
	module->hw = hw;

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	uint32_t sercom_index = _sercom_get_sercom_inst_index(module->hw);
	uint32_t pm_index, gclk_index;

#if (SAML22) || (SAMC20)
	pm_index     = sercom_index + MCLK_APBCMASK_SERCOM0_Pos;
	gclk_index   = sercom_index + SERCOM0_GCLK_ID_CORE;
#elif (SAML21) || (SAMR30) || (SAMR34) || (SAMR35) || (WLR089)
	if (sercom_index == 5) {
		pm_index     = MCLK_APBDMASK_SERCOM5_Pos;
		gclk_index   = SERCOM5_GCLK_ID_CORE;
	} else {
		pm_index     = sercom_index + MCLK_APBCMASK_SERCOM0_Pos;
		gclk_index   = sercom_index + SERCOM0_GCLK_ID_CORE;
	}
#elif (SAMC21)
	pm_index     = sercom_index + MCLK_APBCMASK_SERCOM0_Pos;
	if (sercom_index == 5) {
		gclk_index   = SERCOM5_GCLK_ID_CORE;
	} else {
		gclk_index   = sercom_index + SERCOM0_GCLK_ID_CORE;
	}
#else
	pm_index     = sercom_index + PM_APBCMASK_SERCOM0_Pos;
	gclk_index   = sercom_index + SERCOM0_GCLK_ID_CORE;
#endif

	/* Turn on module in PM */
#if (SAML21) || (SAMR30) || (SAMR34) || (SAMR35) || (WLR089)
	if (sercom_index == 5) {
		system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBD, 1 << pm_index);
	} else {
		system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, 1 << pm_index);
	}
#else
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, 1 << pm_index);
#endif

	/* Set up the GCLK for the module */
	struct system_gclk_chan_config gclk_chan_conf;
	system_gclk_chan_get_config_defaults(&gclk_chan_conf);
	gclk_chan_conf.source_generator = config->generator_source;
	system_gclk_chan_set_config(gclk_index, &gclk_chan_conf);
	system_gclk_chan_enable(gclk_index);
	sercom_set_gclk_generator(config->generator_source, false);

	/* Check if module is enabled. */
	if (i2c_module->CTRLA.reg & SERCOM_I2CM_CTRLA_ENABLE) {
		return STATUS_ERR_DENIED;
	}

	/* Check if reset is in progress. */
	if (i2c_module->CTRLA.reg & SERCOM_I2CM_CTRLA_SWRST) {
		return STATUS_BUSY;
	}

#if I2C_MASTER_CALLBACK_MODE == true
	/* Get sercom instance index and register callback. */
	uint8_t instance_index = _sercom_get_sercom_inst_index(module->hw);
	_sercom_set_handler(instance_index, _i2c_master_interrupt_handler);
	_sercom_instances[instance_index] = module;

	/* Initialize values in module. */
	module->registered_callback = 0;
	module->enabled_callback = 0;
	module->buffer_length = 0;
	module->buffer_remaining = 0;

	module->status = STATUS_OK;
	module->buffer = NULL;
#endif

	/* Set sercom module to operate in I2C master mode. */
	i2c_module->CTRLA.reg = SERCOM_I2CM_CTRLA_MODE(0x5);

	/* Set config and return status. */
	return _i2c_master_set_config(module, config);
}

/**
 * \brief Resets the hardware module
 *
 * Reset the module to hardware defaults.
 *
 * \param[in,out] module Pointer to software module structure
 */
void i2c_master_reset(struct i2c_master_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Wait for sync */
	_i2c_master_wait_for_sync(module);

	/* Disable module */
	i2c_master_disable(module);

#if I2C_MASTER_CALLBACK_MODE == true
	/* Clear all pending interrupts */
	system_interrupt_enter_critical_section();
	system_interrupt_clear_pending(_sercom_get_interrupt_vector(module->hw));
	system_interrupt_leave_critical_section();
#endif

	/* Wait for sync */
	_i2c_master_wait_for_sync(module);

	/* Reset module */
	i2c_module->CTRLA.reg = SERCOM_I2CM_CTRLA_SWRST;
}

#if !defined(__DOXYGEN__)
/**
 * \internal
 * Address response. Called when address is answered or timed out.
 *
 * \param[in,out] module  Pointer to software module structure
 *
 * \return Status of address response.
 * \retval STATUS_OK                    No error has occurred
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
enum status_code _i2c_master_address_response(
		struct i2c_master_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Check for error and ignore bus-error; workaround for BUSSTATE stuck in
	 * BUSY */
	if (i2c_module->INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB) {

		/* Clear write interrupt flag */
		i2c_module->INTFLAG.reg = SERCOM_I2CM_INTFLAG_SB;

		/* Check arbitration. */
		if (i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_ARBLOST) {
			/* Return packet collision. */
			return STATUS_ERR_PACKET_COLLISION;
		}
	/* Check that slave responded with ack. */
	} else if (i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_RXNACK) {
		/* Slave busy. Issue ack and stop command. */
		i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

		/* Return bad address value. */
		return STATUS_ERR_BAD_ADDRESS;
	}

	return STATUS_OK;
}

/**
 * \internal
 * Waits for answer on bus.
 *
 * \param[in,out] module  Pointer to software module structure
 *
 * \return Status of bus.
 * \retval STATUS_OK           If given response from slave device
 * \retval STATUS_ERR_TIMEOUT  If no response was given within specified timeout
 *                             period
 */
enum status_code _i2c_master_wait_for_bus(
		struct i2c_master_module *const module)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Wait for reply. */
	uint16_t timeout_counter = 0;
	while (!(i2c_module->INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB) &&
			!(i2c_module->INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB)) {

		/* Check timeout condition. */
		if (++timeout_counter >= module->buffer_timeout) {
			return STATUS_ERR_TIMEOUT;
		}
	}
	return STATUS_OK;
}
#endif /* __DOXYGEN__ */

/**
 * \internal
 * Send master code for high speed transfer.
 *
 * \param[in,out] module  Pointer to software module structure
 * \param[in]     hs_master_code 8-bit master code (0000 1XXX)
 *
 * \return Status of bus.
 * \retval STATUS_OK           No error happen
 */
enum status_code _i2c_master_send_hs_master_code(
		struct i2c_master_module *const module,
		uint8_t hs_master_code)
{
	SercomI2cm *const i2c_module = &(module->hw->I2CM);
	/* Return value. */
	enum status_code tmp_status;

	/* Set NACK for high speed code */
	i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	/* Send high speed code */
	i2c_module->ADDR.reg = hs_master_code;
	/* Wait for response on bus. */
	tmp_status = _i2c_master_wait_for_bus(module);
	/* Clear write interrupt flag */
	i2c_module->INTFLAG.reg = SERCOM_I2CM_INTENCLR_MB;

	return tmp_status;
}


/**
 * \internal
 * Starts blocking read operation.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of reading packet.
 * \retval STATUS_OK                    The packet was read successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 *
 */
static enum status_code _i2c_master_read_packet(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check arguments */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Return value. */
	enum status_code tmp_status;
	uint16_t tmp_data_length = packet->data_length;

	/* Written buffer counter. */
	uint16_t counter = 0;

	bool sclsm_flag = i2c_module->CTRLA.bit.SCLSM;

	/* Switch to high speed mode */
	if (packet->high_speed) {
		_i2c_master_send_hs_master_code(module, packet->hs_master_code);
	}

	/* Set action to ACK. */
	i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

	/* Set address and direction bit. Will send start command on bus. */
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
		i2c_module->ADDR.reg = (packet->address << 1) | I2C_TRANSFER_READ |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos);
	}

	/* Wait for response on bus. */
	tmp_status = _i2c_master_wait_for_bus(module);

	/* Set action to ack or nack. */
	if ((sclsm_flag) && (packet->data_length == 1)) {
		i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	} else {
		i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;	
	}

	/* Check for address response error unless previous error is
	 * detected. */
	if (tmp_status == STATUS_OK) {
		tmp_status = _i2c_master_address_response(module);
	}

	/* Check that no error has occurred. */
	if (tmp_status == STATUS_OK) {
		/* Read data buffer. */
		while (tmp_data_length--) {
			/* Check that bus ownership is not lost. */
			if (!(i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(2))) {
				return STATUS_ERR_PACKET_COLLISION;
			}

			if (module->send_nack && (((!sclsm_flag) && (tmp_data_length == 0)) ||
					((sclsm_flag) && (tmp_data_length == 1)))) {
				/* Set action to NACK */
				i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
			} else {
				/* Save data to buffer. */
				_i2c_master_wait_for_sync(module);
				packet->data[counter++] = i2c_module->DATA.reg;
				/* Wait for response. */
				tmp_status = _i2c_master_wait_for_bus(module);
			}

			/* Check for error. */
			if (tmp_status != STATUS_OK) {
				break;
			}
		}

		if (module->send_stop) {
			/* Send stop command unless arbitration is lost. */
			_i2c_master_wait_for_sync(module);
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		}

		/* Save last data to buffer. */
		_i2c_master_wait_for_sync(module);
		packet->data[counter] = i2c_module->DATA.reg;
	}

	return tmp_status;
}

/**
 * \brief Reads data packet from slave
 *
 * Reads a data packet from the specified slave address on the I<SUP>2</SUP>C
 * bus and sends a stop condition when finished.
 *
 * \note This will stall the device from any other operation. For
 *       interrupt-driven operation, see \ref i2c_master_read_packet_job.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of reading packet.
 * \retval STATUS_OK                    The packet was read successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
enum status_code i2c_master_read_packet_wait(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

#if I2C_MASTER_CALLBACK_MODE == true
	/* Check if the I2C module is busy with a job. */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}
#endif

	module->send_stop = true;
	module->send_nack = true;

	return _i2c_master_read_packet(module, packet);
}

/**
 * \brief Reads data packet from slave without sending a stop condition when done
 *
 * Reads a data packet from the specified slave address on the I<SUP>2</SUP>C
 * bus without sending a stop condition when done, thus retaining ownership of
 * the bus when done. To end the transaction, a
 * \ref i2c_master_read_packet_wait "read" or
 * \ref i2c_master_write_packet_wait "write" with stop condition must be
 * performed.
 *
 * \note This will stall the device from any other operation. For
 *       interrupt-driven operation, see \ref i2c_master_read_packet_job.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of reading packet.
 * \retval STATUS_OK                    The packet was read successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
enum status_code i2c_master_read_packet_wait_no_stop(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

#if I2C_MASTER_CALLBACK_MODE == true
	/* Check if the I2C module is busy with a job. */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}
#endif

	module->send_stop = false;
	module->send_nack = true;

	return _i2c_master_read_packet(module, packet);
}

/**
 * \internal
 * Starts blocking read operation.
 * \brief Reads data packet from slave without sending a nack signal and a stop
 * condition when done
 *
 * Reads a data packet from the specified slave address on the I<SUP>2</SUP>C
 * bus without sending a nack signal and a stop condition when done,
 * thus retaining ownership of the bus when done. To end the transaction, a
 * \ref i2c_master_read_packet_wait "read" or
 * \ref i2c_master_write_packet_wait "write" with stop condition must be
 * performed.
 *
 * \note This will stall the device from any other operation. For
 *       interrupt-driven operation, see \ref i2c_master_read_packet_job.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of reading packet.
 * \retval STATUS_OK                    The packet was read successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
enum status_code i2c_master_read_packet_wait_no_nack(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

#if I2C_MASTER_CALLBACK_MODE == true
	/* Check if the I2C module is busy with a job. */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}
#endif

	module->send_stop = false;
	module->send_nack = false;

	return _i2c_master_read_packet(module, packet);
}

/**
 * \internal
 * Starts blocking write operation.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of write packet.
 * \retval STATUS_OK                    The packet was write successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
static enum status_code _i2c_master_write_packet(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Return value. */
	enum status_code tmp_status;
	uint16_t tmp_data_length = packet->data_length;

	_i2c_master_wait_for_sync(module);

	/* Switch to high speed mode */
	if (packet->high_speed) {
		_i2c_master_send_hs_master_code(module, packet->hs_master_code);
	}

	/* Set action to ACK. */
	i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

	/* Set address and direction bit. Will send start command on bus. */
	if (packet->ten_bit_address) {
		i2c_module->ADDR.reg = (packet->address << 1) | I2C_TRANSFER_WRITE |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos) |
			SERCOM_I2CM_ADDR_TENBITEN;
	} else {
		i2c_module->ADDR.reg = (packet->address << 1) | I2C_TRANSFER_WRITE |
			(packet->high_speed << SERCOM_I2CM_ADDR_HS_Pos);
	}
	/* Wait for response on bus. */
	tmp_status = _i2c_master_wait_for_bus(module);

	/* Check for address response error unless previous error is
	 * detected. */
	if (tmp_status == STATUS_OK) {
		tmp_status = _i2c_master_address_response(module);
	}

	/* Check that no error has occurred. */
	if (tmp_status == STATUS_OK) {
		/* Buffer counter. */
		uint16_t buffer_counter = 0;

		/* Write data buffer. */
		while (tmp_data_length--) {
			/* Check that bus ownership is not lost. */
			if (!(i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(2))) {
				return STATUS_ERR_PACKET_COLLISION;
			}

			/* Write byte to slave. */
			_i2c_master_wait_for_sync(module);
			i2c_module->DATA.reg = packet->data[buffer_counter++];

			/* Wait for response. */
			tmp_status = _i2c_master_wait_for_bus(module);

			/* Check for error. */
			if (tmp_status != STATUS_OK) {
				break;
			}

			/* Check for NACK from slave. */
			if (i2c_module->STATUS.reg & SERCOM_I2CM_STATUS_RXNACK) {
				/* Return bad data value. */
				tmp_status = STATUS_ERR_OVERFLOW;
				break;
			}
		}

		if (module->send_stop) {
			/* Stop command */
			_i2c_master_wait_for_sync(module);
			i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		}
	}

	return tmp_status;
}

/**
 * \brief Writes data packet to slave
 *
 * Writes a data packet to the specified slave address on the I<SUP>2</SUP>C bus
 * and sends a stop condition when finished.
 *
 * \note This will stall the device from any other operation. For
 *       interrupt-driven operation, see \ref i2c_master_read_packet_job.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of write packet.
 * \retval STATUS_OK                    If packet was write successfully
 * \retval STATUS_BUSY                  If master module is busy with a job
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 * \retval STATUS_ERR_TIMEOUT           If timeout occurred
 * \retval STATUS_ERR_OVERFLOW          If slave did not acknowledge last sent
 *                                      data, indicating that slave does not
 *                                      want more data and was not able to read
 *                                      last data sent
 */
enum status_code i2c_master_write_packet_wait(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

#if I2C_MASTER_CALLBACK_MODE == true
	/* Check if the I2C module is busy with a job */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}
#endif

	module->send_stop = true;
	module->send_nack = true;

	return _i2c_master_write_packet(module, packet);
}

/**
 * \brief Writes data packet to slave without sending a stop condition when done
 *
 * Writes a data packet to the specified slave address on the I<SUP>2</SUP>C bus
 * without sending a stop condition, thus retaining ownership of the bus when
 * done. To end the transaction, a \ref i2c_master_read_packet_wait "read" or
 * \ref i2c_master_write_packet_wait "write" with stop condition or sending a
 * stop with the \ref i2c_master_send_stop function must be performed.
 *
 * \note This will stall the device from any other operation. For
 *       interrupt-driven operation, see \ref i2c_master_read_packet_job.
 *
 * \param[in,out] module  Pointer to software module struct
 * \param[in,out] packet  Pointer to I<SUP>2</SUP>C packet to transfer
 *
 * \return Status of write packet.
 * \retval STATUS_OK                    If packet was write successfully
 * \retval STATUS_BUSY                  If master module is busy
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 * \retval STATUS_ERR_TIMEOUT           If timeout occurred
 * \retval STATUS_ERR_OVERFLOW          If slave did not acknowledge last sent
 *                                      data, indicating that slave do not want
 *                                      more data
 */
enum status_code i2c_master_write_packet_wait_no_stop(
		struct i2c_master_module *const module,
		struct i2c_master_packet *const packet)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);
	Assert(packet);

#if I2C_MASTER_CALLBACK_MODE == true
	/* Check if the I2C module is busy with a job */
	if (module->buffer_remaining > 0) {
		return STATUS_BUSY;
	}
#endif

	module->send_stop = false;
	module->send_nack = true;

	return _i2c_master_write_packet(module, packet);
}

/**
 * \brief Sends stop condition on bus
 *
 * Sends a stop condition on bus.
 *
 * \note This function can only be used after the
 *       \ref i2c_master_write_packet_wait_no_stop function. If a stop condition
 *       is to be sent after a read, the \ref i2c_master_read_packet_wait
 *       function must be used.
 *
 * \param[in,out] module  Pointer to the software instance struct
 */
void i2c_master_send_stop(struct i2c_master_module *const module)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Send stop command */
	_i2c_master_wait_for_sync(module);
	i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
}

/**
 * \brief Sends nack signal on bus
 *
 * Sends a nack signal on bus.
 *
 * \note This function can only be used after the
 *       \ref i2c_master_write_packet_wait_no_nack function,
 *        or \ref i2c_master_read_byte function.
 * \param[in,out] module  Pointer to the software instance struct
 */
void i2c_master_send_nack(struct i2c_master_module *const module)
{
	/* Sanity check */
	Assert(module);
	Assert(module->hw);

	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Send nack signal */
	_i2c_master_wait_for_sync(module);
	i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
}

/**
 * \brief Reads one byte data from slave
 *
 * \param[in,out]  module  Pointer to software module struct
 * \param[out]     byte    Read one byte data to slave
 *
 * \return Status of reading byte.
 * \retval STATUS_OK                    One byte was read successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
enum status_code i2c_master_read_byte(
		struct i2c_master_module *const module,
		uint8_t *byte)
{
  	enum status_code tmp_status;
  	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	i2c_module->CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	/* Write byte to slave. */
	_i2c_master_wait_for_sync(module);
	*byte = i2c_module->DATA.reg;
	/* Wait for response. */
	tmp_status = _i2c_master_wait_for_bus(module);

	return tmp_status;
}

/**
 * \brief Write one byte data to slave
 *
 * \param[in,out]  module  Pointer to software module struct
 * \param[in]      byte    Send one byte data to slave
 *
 * \return Status of writing byte.
 * \retval STATUS_OK                    One byte was write successfully
 * \retval STATUS_ERR_TIMEOUT           If no response was given within
 *                                      specified timeout period
 * \retval STATUS_ERR_DENIED            If error on bus
 * \retval STATUS_ERR_PACKET_COLLISION  If arbitration is lost
 * \retval STATUS_ERR_BAD_ADDRESS       If slave is busy, or no slave
 *                                      acknowledged the address
 */
enum status_code i2c_master_write_byte(
		struct i2c_master_module *const module,
		uint8_t byte)
{
  	enum status_code tmp_status;
  	SercomI2cm *const i2c_module = &(module->hw->I2CM);

	/* Write byte to slave. */
	_i2c_master_wait_for_sync(module);
	i2c_module->DATA.reg = byte;
	/* Wait for response. */
	tmp_status = _i2c_master_wait_for_bus(module);
	return tmp_status;
}

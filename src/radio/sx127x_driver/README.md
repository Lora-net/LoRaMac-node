# SX127X driver

This package proposes an implementation in C of the driver for **SX127X** radio component.

## Structure

The driver is defined as follows:

- sx127x.c: implementation of the driver functions
- sx127x.h: declarations of the driver functions
- sx127x_regs_common_defs.h: definitions of all common reegisters (address and fields)
- sx127x_regs_gfsk_defs.h: definitions of all GFSK reegisters (address and fields)
- sx127x_regs_lora_defs.h: definitions of all LoRa reegisters (address and fields)
- sx127x_regs.h: includes sx127x_regs_common_defs.h, sx127x_regs_gfsk_defs.h and sx127x_regs_lora_defs.h
- sx127x_hal.h: declarations of the HAL functions (to be implemented by the user - see below)

## HAL

The HAL (Hardware Abstraction Layer) is a collection of functions the user shall implement to write platform-dependant calls to the host. The list of functions is the following:

- sx127x_hal_get_radio_id
- sx127x_hal_dio_irq_attach
- sx127x_hal_write
- sx127x_hal_read
- sx127x_hal_reset
- sx127x_hal_get_dio_1_pin_state
- sx127x_hal_timer_start
- sx127x_hal_timer_stop
- sx127x_hal_timer_is_started
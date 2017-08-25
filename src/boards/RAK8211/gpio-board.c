#include "board.h"
#include "nrf_drv_gpiote.h"

static GpioIrqHandler* irq_handlers[32];

static void pin_event_handler( nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action )
{
	if (irq_handlers[pin] != NULL)
		irq_handlers[pin]();
}

void GpioInit( Gpio_t *obj, PinNames pin, PinModes mode,  PinConfigs config, PinTypes type, uint32_t value )
{
	ret_code_t err_code;

	if (obj->port == NULL)
	{
		obj->pin = pin;
		obj->mode = mode;
		obj->pull = type;
		obj->irq_mode = NO_IRQ;

		if (mode == PIN_INPUT)
		{
			nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
			if (type == PIN_PULL_UP)
				config.pull = NRF_GPIO_PIN_PULLUP;
			else if (type == PIN_PULL_DOWN)
				config.pull = NRF_GPIO_PIN_PULLDOWN;
			err_code = nrf_drv_gpiote_in_init(pin, &config, pin_event_handler);
		}
		else
		{
			nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_SIMPLE(false);
			if (value != 0)
				config.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
			err_code = nrf_drv_gpiote_out_init(pin, &config);
		}
		APP_ERROR_CHECK(err_code);
		if (err_code == NRF_SUCCESS)
		{
			obj->port = obj;
		}
	}
	else
	{
		GpioWrite( obj, value );
	}
}

void GpioSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
	ret_code_t err_code;

	if (obj->mode != PIN_INPUT)
		return;

	if (obj->port != NULL)
	{
		nrf_drv_gpiote_in_uninit(obj->pin);
		obj->port = NULL;
	}

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

	obj->irq_mode = irqMode;
	if (irqMode == IRQ_RISING_EDGE)
		config.sense = NRF_GPIOTE_POLARITY_LOTOHI;
	else if (irqMode == IRQ_FALLING_EDGE)
		config.sense = NRF_GPIOTE_POLARITY_HITOLO;

	if (obj->pull == PIN_PULL_UP)
		config.pull = NRF_GPIO_PIN_PULLUP;
	else if (obj->pull == PIN_PULL_DOWN)
		config.pull = NRF_GPIO_PIN_PULLDOWN;

	err_code = nrf_drv_gpiote_in_init(obj->pin, &config, pin_event_handler);
	if (err_code != NRF_SUCCESS)
		return;

	irq_handlers[obj->pin] = irqHandler;
	nrf_drv_gpiote_in_event_enable(obj->pin, true);
	obj->port = obj;
}

void GpioRemoveInterrupt( Gpio_t *obj )
{
	if ( obj->mode != PIN_INPUT || obj->port == NULL)
		return;
	nrf_drv_gpiote_in_event_disable( obj->pin );
}

void GpioWrite( Gpio_t *obj, uint32_t value )
{
	if ( obj->mode != PIN_OUTPUT || obj->port == NULL )
		return;

	if ( value )
		nrf_drv_gpiote_out_set(obj->pin);
	else
		nrf_drv_gpiote_out_clear( obj->pin );
}

void GpioToggle( Gpio_t *obj )
{
	if ( obj->mode != PIN_OUTPUT || obj->port == NULL )
		return;

	nrf_drv_gpiote_out_toggle(obj->pin);
}

uint32_t GpioRead( Gpio_t *obj )
{
	if ( obj->mode != PIN_INPUT || obj->port == NULL )
		return 0;

	if ( nrf_drv_gpiote_in_is_set( obj->pin ))
		return 1;
	return 0;
}

void GpioDeinit( Gpio_t *obj )
{
	if (obj->port == NULL)
		return;

	if (obj->mode == PIN_INPUT)
		nrf_drv_gpiote_in_uninit(obj->pin);
	else if (obj->mode == PIN_OUTPUT)
		nrf_drv_gpiote_out_uninit(obj->pin);

	obj->port = NULL;
}

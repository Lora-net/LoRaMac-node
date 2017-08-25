#include "board.h"
#include "nrf_drv_spi.h"
#include <string.h>

void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
	obj->Instance.p_registers = NRF_SPI0;
	obj->Instance.irq = SPI0_IRQ;
	obj->Instance.drv_inst_idx = SPI0_INSTANCE_INDEX;
	obj->Instance.use_easy_dma = SPI0_USE_EASY_DMA;

	obj->Config.sck_pin			= sclk;
	obj->Config.mosi_pin		= mosi;
	obj->Config.miso_pin		= miso;
	obj->Config.ss_pin			= nss;
	obj->Config.irq_priority	= SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
	obj->Config.orc				= 0xFF;
	obj->Config.mode			= NRF_DRV_SPI_MODE_0;
	obj->Config.bit_order		= NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
	obj->Config.frequency		= NRF_DRV_SPI_FREQ_1M;

	ret_code_t error_code = nrf_drv_spi_init(&obj->Instance, &obj->Config, NULL, NULL);
	APP_ERROR_CHECK(error_code);
	if ( NRF_SUCCESS == error_code )
		obj->initialized = true;
}

void SpiDeInit( Spi_t *obj )
{
	if (obj->initialized)
	{
		nrf_drv_spi_uninit(&obj->Instance);
		obj->initialized = false;
	}
}

void SpiOut( Spi_t *obj, uint8_t addr, uint8_t *outData, uint8_t outLen )
{
	if (obj->initialized != true)
		return;

	uint16_t length = outLen + 1;
	uint8_t tx[length];
	tx[0] = addr | 0x80;
	if ( outLen == 1 )
		tx[1] = *outData;
	else
		memcpy(&tx[1], outData, outLen);

    BoardDisableIrq( );

	nrf_drv_spi_transfer(&obj->Instance, tx, length, NULL, 0);

    BoardEnableIrq( );
}

void SpiIn( Spi_t *obj, uint8_t addr, uint8_t *inData, uint8_t inLen )
{
	if (obj->initialized != true)
		return;

	uint8_t tx[1];
	uint8_t rx[inLen + 1];
	tx[0] = addr & 0x7F;

    BoardDisableIrq( );

	nrf_drv_spi_transfer(&obj->Instance, tx, 1, rx, inLen + 1);

    BoardEnableIrq( );

	if ( inLen == 1 )
		*inData = rx[1];
	else
		memcpy(inData, &rx[1], inLen);
}

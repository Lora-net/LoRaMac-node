/**
* \file  radio_driver_hal.c
*
* \brief This is the Radio Driver HAL source file which 
*        contains LoRa-specific Radio Driver Hardware Abstract Layer
*		
*
* Copyright (c) 2019 Microchip Technology Inc. and its subsidiaries. 
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
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, 
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
#include "asf.h"
#include "radio_driver_hal.h"
#include "board.h"
#include "spi.h"
#include "sys.h"
#ifdef CONF_PMM_ENABLE
#include "pmm.h"
#endif

/******************************************************************************/
/*  Defines                                                                   */
/******************************************************************************/
#ifndef RADIO_CLK_STABILITATION_DELAY
/* Delay in ms for Radio clock source to stabilize */
#define RADIO_CLK_STABILITATION_DELAY      0
#endif

#ifndef RADIO_CLK_SRC
/* Clock source for SX1276 radio */
#define RADIO_CLK_SRC                      XTAL
#endif

/*Enable the required DIO's*/
#define ENABLE_DIO0
#define ENABLE_DIO1
#define ENABLE_DIO2

/*************************************** RADIO Callbacks***********************/
#ifdef ENABLE_DIO0
static DioInterruptHandler_t interruptHandlerDio0 = NULL;
#endif

#ifdef ENABLE_DIO1
static DioInterruptHandler_t interruptHandlerDio1 = NULL;
#endif

#ifdef ENABLE_DIO2
static DioInterruptHandler_t interruptHandlerDio2 = NULL;
#endif

#ifdef ENABLE_DIO3
static DioInterruptHandler_t interruptHandlerDio3 = NULL;
#endif

#ifdef ENABLE_DIO4
static DioInterruptHandler_t interruptHandlerDio4 = NULL;
#endif

#ifdef ENABLE_DIO5
static DioInterruptHandler_t interruptHandlerDio5 = NULL;
#endif

/*************************************** Prototypes ***************************/

/*
 * \brief Initializes the Radio SPI Interface
 */
static void HAL_RadioSpiInit(void);

/*
 * \brief Initializes the Radio Digital IO's,Reset Pins
 */ 
static void HAL_RadioIOInit(void);

/*
 * \brief The below functions are callbacks for the TRX DIO interrupts
 */
#ifdef ENABLE_DIO0
static void HAL_RadioDIO0Callback(void);
#endif
#ifdef ENABLE_DIO1
static void HAL_RadioDIO1Callback(void);
#endif
#ifdef ENABLE_DIO2
static void HAL_RadioDIO2Callback(void);
#endif
#ifdef ENABLE_DIO3
static void HAL_RadioDIO3Callback(void);
#endif
#ifdef ENABLE_DIO4
static void HAL_RadioDIO4Callback(void);
#endif
#ifdef ENABLE_DIO5
static void HAL_RadioDIO5Callback(void);
#endif

/*
 * \brief This function is used to make the Reset Pin as Output Pin
 */
static void HAL_ResetPinMakeOutput(void);


/*
 * \brief This function is used to make the Reset Pin as Input Pin
 */
static void HAL_ResetPinMakeInput(void);


/*
 * \brief This function is used to set/clear the Radio Reset Pin
 * \param[in] value Set/Clear the Radio Reset pin 
 */
static void HAL_ResetPinOutputValue(uint8_t value);


/*
 * \brief This function is used to write a byte of data to the SPI DATA register
 * \param[in] data Byte of data to be written into the SPI DATA register
 */
static uint8_t HAL_SPISend(uint8_t data);

/*
 * \brief This function is called to select a SPI slave
 */
static void HAL_SPICSAssert(void);

/*
 * \brief This function is called to deselect a SPI slave
 */
static void HAL_SPICSDeassert(void);

/***************************************** GLOBALS ***************************/
static struct spi_module master;
struct spi_slave_inst slave;
static uint8_t dioStatus;	

/***************************************** MACROS *****************************/
/*The SPI Baud rate needs to be defined in conf_board.h*/

#ifndef SX_RF_SPI_BAUDRATE
/* TODO - Change the SPI baudrate according to MCU clock frequency */
#define SX_RF_SPI_BAUDRATE 2000000
#endif

/*********************************** Implementation***************************/

/** 
 * \brief This function is used to initialize the Radio Hardware
 * The SPI interface,DIO and reset pins are initialized by this api
 */
void HAL_RadioInit(void)
{
	HAL_RadioIOInit();
	HAL_RadioSpiInit();
}
/**
 * \brief This function is used to initialize the SPI Interface after PMM wakeup
 *
 */
void HAL_Radio_resources_init(void)
{
	spi_enable(&master);
	while (spi_is_syncing(&master)) {
		/* Wait until the synchronization is complete */
	}
}
/**
 * \brief This function is used to deinitialize the SPI Interface
 */
void HAL_RadioDeInit(void)
{
	spi_disable(&master);
}
 
/** 
 * \brief This function resets the Radio hardware by pulling the reset pin low
 */
void RADIO_Reset(void)
{
	HAL_ResetPinMakeOutput();
	HAL_ResetPinOutputValue(0);
	SystemBlockingWaitMs(1);
	HAL_ResetPinMakeInput();
		
	//Added these two lines to make sure this pin is not left in floating state during sleep
	HAL_ResetPinOutputValue(1);
	HAL_ResetPinMakeOutput();
}

/** 
 * \brief This function is used to write a byte of data to the radio register
 * \param[in] reg Radio register to be written
 * \param[in] value Value to be written into the radio register
 */
void RADIO_RegisterWrite(uint8_t reg, uint8_t value)
{
	HAL_SPICSAssert();
	HAL_SPISend(REG_WRITE_CMD | reg);
	HAL_SPISend(value);
	HAL_SPICSDeassert();
}

/** 
 * \brief This function is used to read a byte of data from the radio register
 * \param[in] reg Radio register to be read
 * \retval  Value read from the radio register
 */
uint8_t RADIO_RegisterRead(uint8_t reg)
{
	uint8_t readValue;
	reg &= 0x7F;    // Make sure write bit is not set
	HAL_SPICSAssert();
	HAL_SPISend(reg);
	readValue = HAL_SPISend(0xFF);
	HAL_SPICSDeassert();
	return readValue;
}

/** 
 * \brief This function is used to  write a stream of data into the Radio Frame buffer
 * \param[in] FIFO offset to be written to
 * \param[in] buffer Pointer to the data to be written into the frame buffer
 * \param[in] bufferLen Length of the data to be written
 */
void RADIO_FrameWrite(uint8_t offset, uint8_t* buffer, uint8_t bufferLen)
{
    HAL_SPICSAssert();
    HAL_SPISend(REG_WRITE_CMD | offset);

    for (uint8_t i = 0; i < bufferLen; i++)
    {
	    HAL_SPISend(buffer[i]);
    }
    HAL_SPICSDeassert();

}

/** 
 * \brief This function is used to  read a stream of data from the Radio Frame buffer
 * \param[in] FIFO offset to be read from
 * \param[in] buffer Pointer to the data where the data is read and stored
 * \param[in] bufferLen Length of the data to be read from the frame buffer
 */
void RADIO_FrameRead(uint8_t offset, uint8_t* buffer, uint8_t bufferLen)
{
    HAL_SPICSAssert();
    HAL_SPISend(offset);
    for (uint8_t i = 0; i < bufferLen; i++)
    {
	    buffer[i] = HAL_SPISend(0xFF);
    }
    HAL_SPICSDeassert();
}


#ifdef ENABLE_DIO0
/** 
 * \brief This function is used to enable DIO0 interrupt
 */
void HAL_EnableDIO0Interrupt(void)
{
	extint_chan_enable_callback(DIO0_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

/** 
 * \brief This function is used to disable DIO0 interrupt
 */
void HAL_DisbleDIO0Interrupt(void)
{
	extint_chan_disable_callback(DIO0_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

/** 
 * \brief This function is used to read the status of  DIO0 pin
 */
uint8_t HAL_DIO0PinValue(void)
{
	return port_pin_get_input_level(DIO0_EIC_PIN);
}

static void HAL_RadioDIO0Callback(void)
{
  if (interruptHandlerDio0)
  {
#ifdef CONF_PMM_ENABLE
	PMM_Wakeup();
#endif	
    interruptHandlerDio0();
  }    
}
#endif

#ifdef ENABLE_DIO1

/** 
 * \brief This function is used to enable DIO1 interrupt
 */
void HAL_EnableDIO1Interrupt(void)
{
	extint_chan_enable_callback(DIO1_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

/** 
 * \brief This function is used to disable DIO2 interrupt
 */
void HAL_DisbleDIO1Interrupt(void)
{
	extint_chan_disable_callback(DIO1_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to read the status of  DIO1 pin
 */
uint8_t HAL_DIO1PinValue(void)
{
	return port_pin_get_input_level(DIO1_EIC_PIN);
}

static void HAL_RadioDIO1Callback(void)
{
  if (interruptHandlerDio1)
  {
#ifdef CONF_PMM_ENABLE
	PMM_Wakeup();
#endif
    interruptHandlerDio1();
  }
}
#endif

#ifdef ENABLE_DIO2

/** 
 * \brief This function is used to enable DIO2 interrupt
 */
void HAL_EnableDIO2Interrupt(void)
{
	extint_chan_enable_callback(DIO2_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to disable DIO2 interrupt
 */
void HAL_DisbleDIO2Interrupt(void)
{
	extint_chan_disable_callback(DIO2_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to read the status of  DIO2 pin
 */
uint8_t HAL_DIO2PinValue(void)
{
	return port_pin_get_input_level(DIO2_EIC_PIN);
}

static void HAL_RadioDIO2Callback(void)
{
  if (interruptHandlerDio2)
  {
#ifdef CONF_PMM_ENABLE
	PMM_Wakeup();
#endif
    interruptHandlerDio2();
  }
}

#endif

#ifdef ENABLE_DIO3

/** 
 * \brief This function is used to enable DIO3 interrupt
 */
void HAL_EnableDIO3Interrupt(void)
{
	extint_chan_enable_callback(DIO3_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to disable DIO3 interrupt
 */
void HAL_DisbleDIO3Interrupt(void)
{
	extint_chan_disable_callback(DIO3_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to read the status of  DIO3 pin
 */
uint8_t HAL_DIO3PinValue(void)
{
	return port_pin_get_input_level(DIO3_EIC_PIN);
}

static void HAL_RadioDIO3Callback(void)
{
  if (interruptHandlerDio3)
  {
#ifdef CONF_PMM_ENABLE
	PMM_Wakeup();
#endif
    interruptHandlerDio3();
  }
}
#endif

#ifdef ENABLE_DIO4
/** 
 * \brief This function is used to enable DIO4 interrupt
 */
void HAL_EnableDIO4Interrupt(void)
{
	extint_chan_enable_callback(DIO4_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to disable DIO4 interrupt
 */
void HAL_DisbleDIO4Interrupt(void)
{
	extint_chan_disable_callback(DIO4_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

/** 
 * \brief This function is used to read the status of  DIO4 pin
 */
uint8_t HAL_DIO4PinValue(void)
{
	return port_pin_get_input_level(DIO4_EIC_PIN);
}

static void HAL_RadioDIO4Callback(void)
{
  if (interruptHandlerDio4)
  {
#ifdef CONF_PMM_ENABLE
	PMM_Wakeup();
#endif
    interruptHandlerDio4();
  }
}
#endif

#ifdef ENABLE_DIO5

/** 
 * \brief This function is used to enable DIO5 interrupt
 */
void HAL_EnableDIO5Interrupt(void)
{
	extint_chan_enable_callback(DIO5_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
/** 
 * \brief This function is used to disable DIO5 interrupt
 */
void HAL_DisbleDIO5Interrupt(void)
{
	extint_chan_disable_callback(DIO5_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

static void HAL_RadioDIO5Callback(void)
{
  if (interruptHandlerDio5)
  {
#ifdef CONF_PMM_ENABLE
	PMM_Wakeup();
#endif
    interruptHandlerDio5();
  }
}
/** 
 * \brief This function is used to read the status of  DIO5 pin
 */
uint8_t HAL_DIO5PinValue(void)
{
	return port_pin_get_input_level(DIO5_EIC_PIN);
}
#endif

/** 
 * \brief This function is used to get the interrupt status
 * The interrupt status is cleared after calling this function
 * \retval Returns the mask of received interrupts
 */
uint8_t INTERRUPT_GetDioStatus(void)
{
	uint8_t a;
	INTERRUPT_GlobalInterruptDisable();
	a = dioStatus;
	dioStatus = 0;
	INTERRUPT_GlobalInterruptEnable();
	return a;
}

/** 
 * \brief This function is used to get the interrupt status
 * The interrupt status is not cleared after calling this function 
 * \retval Returns the mask of received interrupts
 */
uint8_t INTERRUPT_PeekDioStatus(void)
{
	return dioStatus;
}

/*
 * \brief Initializes the Radio SPI Interface
 */
void HAL_RadioSpiInit(void)
{
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;
	
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	
	slave_dev_config.ss_pin = SX_RF_SPI_CS;
	spi_attach_slave(&slave, &slave_dev_config);
	
	spi_get_config_defaults(&config_spi_master);

	config_spi_master.mode_specific.master.baudrate = SX_RF_SPI_BAUDRATE;
	config_spi_master.mux_setting = SX_RF_SPI_SERCOM_MUX_SETTING;
	config_spi_master.pinmux_pad0 = SX_RF_SPI_SERCOM_PINMUX_PAD0;
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
	config_spi_master.pinmux_pad2 = SX_RF_SPI_SERCOM_PINMUX_PAD2;
	config_spi_master.pinmux_pad3 = SX_RF_SPI_SERCOM_PINMUX_PAD3;
	
	spi_init(&master, SX_RF_SPI, &config_spi_master);	
	spi_enable(&master);
	
}


/*
 * \brief Initializes the Radio Digital IO's,Reset Pins
 */ 
static void HAL_RadioIOInit(void)
{

	struct extint_chan_conf config_extint_chan;
	struct port_config pin_conf;
	
	port_get_config_defaults(&pin_conf);
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	
	port_pin_set_config(SX_RF_SPI_SCK, &pin_conf);
	port_pin_set_config(SX_RF_SPI_MOSI, &pin_conf);
	port_pin_set_config(SX_RF_SPI_CS, &pin_conf);
	port_pin_set_config(SX_RF_RESET_PIN, &pin_conf);
	
	port_pin_set_output_level(SX_RF_SPI_SCK, true);
	port_pin_set_output_level(SX_RF_SPI_MOSI, true);
	port_pin_set_output_level(SX_RF_SPI_CS, true);
	port_pin_set_output_level(SX_RF_RESET_PIN, true);
	
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	port_pin_set_config(SX_RF_SPI_MISO, &pin_conf);
	
	#ifdef ENABLE_DIO0
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = DIO0_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DIO0_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(DIO0_EIC_LINE, &config_extint_chan);
	extint_register_callback(HAL_RadioDIO0Callback,DIO0_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DIO0_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	#endif

	#ifdef ENABLE_DIO1
	config_extint_chan.gpio_pin           = DIO1_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DIO1_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(DIO1_EIC_LINE, &config_extint_chan);
	extint_register_callback(HAL_RadioDIO1Callback,DIO1_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DIO1_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	#endif

	#ifdef ENABLE_DIO2
	config_extint_chan.gpio_pin           = DIO2_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DIO2_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(DIO2_EIC_LINE, &config_extint_chan);
	extint_register_callback(HAL_RadioDIO2Callback,DIO2_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DIO2_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	#endif

	#ifdef ENABLE_DIO3
	config_extint_chan.gpio_pin           = DIO3_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DIO3_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(DIO3_EIC_LINE, &config_extint_chan);
	extint_register_callback(HAL_RadioDIO3Callback,DIO3_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DIO3_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	#endif

	#ifdef ENABLE_DIO4
	config_extint_chan.gpio_pin           = DIO4_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DIO4_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(DIO4_EIC_LINE, &config_extint_chan);
	extint_register_callback(HAL_RadioDIO4Callback,DIO4_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DIO4_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	#endif

	#ifdef ENABLE_DIO5
	config_extint_chan.gpio_pin           = DIO5_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DIO5_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(DIO5_EIC_LINE, &config_extint_chan);
	extint_register_callback(HAL_RadioDIO5Callback,DIO5_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DIO5_EIC_LINE,EXTINT_CALLBACK_TYPE_DETECT);
	#endif
}


/*
 * \brief This function is used to make the Reset Pin as Output Pin
 */

static void HAL_ResetPinMakeOutput(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	pin_conf.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(SX_RF_RESET_PIN,&pin_conf);
}
/*
 * \brief This function is used to make the Reset Pin as Input Pin
 */
static void HAL_ResetPinMakeInput(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	pin_conf.direction = PORT_PIN_DIR_INPUT;
	port_pin_set_config(SX_RF_RESET_PIN,&pin_conf);
}

/*
 * \brief This function is used to set/clear the Radio Reset Pin
 * \param[in] value Set/Clear the Radio Reset pin 
 */
static void HAL_ResetPinOutputValue(uint8_t value)
{
	if (value == 0)
	{
		port_pin_set_output_level(SX_RF_RESET_PIN, SX_RF_RESET_LOW);
	}
	else if (value == 1)
	{
		port_pin_set_output_level(SX_RF_RESET_PIN, SX_RF_RESET_HIGH);
	}
}
/*
 * \brief This function is called to select a SPI slave
 */
static void HAL_SPICSAssert(void)
{
	spi_select_slave(&master, &slave, true);
}

/*
 * \brief This function is called to deselect a SPI slave
 */
static void HAL_SPICSDeassert(void)
{
	spi_select_slave(&master, &slave, false);
}

/*
 * \brief This function is used to write a byte of data to the SPI DATA register
 * \param[in] data Byte of data to be written into the SPI DATA register
 */
static uint8_t HAL_SPISend(uint8_t data)
{
	uint16_t read_val = 0;
	
	/* Write the byte in the transceiver data register */
	while (!spi_is_ready_to_write(&master));
	spi_write(&master, data);
	while (!spi_is_write_complete(&master));
	
	while (!spi_is_ready_to_read(&master));
	spi_read(&master, &read_val);
	
	return ((uint8_t)read_val);
}

/**
 * \brief This function sets the interrupt handler for given DIO interrupt
 *
 * \param[in] dioPin  - DIO pin
 * \param[in] handler - function to be called upon given DIO interrupt
 */
void HAL_RegisterDioInterruptHandler(uint8_t dioPin, DioInterruptHandler_t handler)
{
  switch (dioPin)
  {
#ifdef ENABLE_DIO0
    case DIO0:
      interruptHandlerDio0 = handler;
      break;
#endif

#ifdef ENABLE_DIO1
    case DIO1:
      interruptHandlerDio1 = handler;
      break;
#endif

#ifdef ENABLE_DIO2
    case DIO2:
      interruptHandlerDio2 = handler;
      break;
#endif

#ifdef ENABLE_DIO3
    case DIO3:
      interruptHandlerDio3 = handler;
      break;
#endif

#ifdef ENABLE_DIO4
    case DIO4:
      interruptHandlerDio4 = handler;
      break;
#endif

#ifdef ENABLE_DIO5
    case DIO5:
      interruptHandlerDio5 = handler;
      break;
#endif

    default:
      break;
  }
}


/**
 * \brief This function Enables RF Control pins
 *
 * \param[in] RFCtrl1
 * 				RFO_LF = 0
 *				RFO_HF = 1
 *				PA_BOOST = 2
 * \param[in] RFCtrl2
 *				RX = 0
 *				TX = 1 
 * \param[out] None
 */

void HAL_EnableRFCtrl(RFCtrl1_t RFCtrl1, RFCtrl2_t RFCtrl2)
{
	/* In standard SAMR34_XPRO, Only RFO_HF and PA_BOOST needs to be controlled by a GPIO pin */
#ifdef RFSWITCH_ENABLE
   if ((RFCtrl1 == RFO_HF) || (RFCtrl2 == RX))
   {
		port_pin_set_output_level(RF_SWITCH_PIN, RF_SWITCH_ACTIVE);		
   }
   else if ((RFCtrl1 == PA_BOOST) && (RFCtrl2 == TX))
   {
	   port_pin_set_output_level(RF_SWITCH_PIN, RF_SWITCH_INACTIVE);
   }
#endif	
}

/**
 * \brief This function Disables RF Control pins
 *
 * \param[in] RFCtrl1
 * 				RFO_LF = 0
 *				RFO_HF = 1
 *				PA_BOOST = 2
 * \param[in] RFCtrl2
 *				RX = 0
 *				TX = 1 
 * \param[out] None
 */

void HAL_DisableRFCtrl(RFCtrl1_t RFCtrl1, RFCtrl2_t RFCtrl2)
{
	/* In standard SAMR34_XPRO, Only RFO_HF and PA_BOOST needs to be controlled by a GPIO pin */
#ifdef RFSWITCH_ENABLE
	if ((RFCtrl1 == RFO_HF) || (RFCtrl2 == RX))
	{
		port_pin_set_output_level(RF_SWITCH_PIN, RF_SWITCH_INACTIVE);	
	}
#endif	
}

/**
 * \brief This function gets the time taken for the radio clock to stabilize
 *
 * \param[in] None
 * \param[out] Time value in ms
 */
uint8_t HAL_GetRadioClkStabilizationDelay(void)
{
	return RADIO_CLK_STABILITATION_DELAY;
}

/**
 * \brief This function gets the clock source of Radio defined by user
 *
 * \param[in] None
 * \param[out] Type of clock source TCXO or XTAL
 */
RadioClockSources_t HAL_GetRadioClkSrc(void)
{
	return RADIO_CLK_SRC;
}

/**
 * \brief This function Powering up the TCXO oscillator
 *
 * \param[in] None
 * \param[out] None
 */
void HAL_TCXOPowerOn(void)
{
#ifdef TCXO_ENABLE
	port_pin_set_output_level(TCXO_PWR_PIN, TCXO_PWR_ACTIVE);
	delay_ms(RADIO_CLK_STABILITATION_DELAY);
#endif
}

/**
 * \brief This function Powering off the TCXO oscillator
 *
 * \param[in] None
 * \param[out] None
 */
void HAL_TCXOPowerOff(void)
{
#ifdef TCXO_ENABLE
#ifndef TCXO_ALWAYS_ON
	port_pin_set_output_level(TCXO_PWR_PIN, TCXO_PWR_INACTIVE);
#endif
#endif
}
/**
 End of File
*/

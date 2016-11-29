/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Board ADC driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __ADC_MCU_H__
#define __ADC_MCU_H__

/*!
 * \brief Initializes the ADC object and MCU peripheral
 *
 * \param [IN] obj      ADC object
 * \param [IN] adcInput ADC input pin
 */
void AdcMcuInit( Adc_t *obj, PinNames adcInput );

/*!
 * \brief Initializes the ADC internal parameters
 */
void AdcMcuConfig( void );

/*!
 * \brief Reads the value of the given channel
 *
 * \param [IN] obj     ADC object
 * \param [IN] channel ADC input channel
 */
uint16_t AdcMcuReadChannel( Adc_t *obj, uint32_t channel );

#endif // __ADC_MCU_H__

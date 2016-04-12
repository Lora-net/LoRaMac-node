/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Board ADC driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __ADC_MCU_H__
#define __ADC_MCU_H__

/*!
 * Register the old AdcMcuReadChannel function to the new function 
 * which makes an additional parameter available to select the read out channel
 */
#define AdcMcuReadChannel( obj ) AdcMcuRead( obj, ADC_CHANNEL_8 )

/*!
 * ADC resolution
 */
typedef enum
{
    ADC_12_BIT = 0,
    ADC_10_BIT,
    ADC_8_BIT,
    ADC_6_BIT
}AdcResolution;

/*!
 * ADC conversion trigger
 */
typedef enum
{
    CONVERT_MANUAL_TRIG = 0,
    CONVERT_RISING_EDGE,
    CONVERT_FALLING_EDGE,
    CONVERT_RISING_FALLING_EDGE
}AdcTriggerConv;

/*!
 * ADC data alignment 
 */
typedef enum
{
    DATA_RIGHT_ALIGNED = 0,
    DATA_LEFT_ALIGNED
}AdcDataAlignement;


/*!
 * ADC conversion mode
 */
typedef enum
{
    SINGLE_CONVERSION = 0,
    CONTIMUOUS_CONVERSION
}AdcNumConversion;

/*!
 * \brief Initializes the ADC object and MCU peripheral
 *
 * \param [IN] obj  ADC object
 * \param [IN] scl  ADC input pin
 */
void AdcMcuInit( Adc_t *obj, PinNames adcInput );

/*!
 * \brief DeInitializes the ADC object and MCU peripheral
 *
 * \param [IN] obj  ADC object
 */
void AdcMcuDeInit( Adc_t *obj );

/*!
 * \brief Initializes the ADC internal parameters
 *
 * \param [IN] obj          ADC object
 * \param [IN] AdcRes       ADC resolution 
 * \param [IN] AdcNumConv   ADC number of conversion
 * \param [IN] AdcTrig      ADC conversion trigger
 * \param [IN] AdcDataAlig  ADC data output alignement
 */
void AdcMcuFormat( Adc_t *obj, AdcResolution AdcRes, AdcNumConversion AdcNumConv, AdcTriggerConv AdcTrig, AdcDataAlignement AdcDataAlig );

uint16_t AdcMcuRead( Adc_t *obj, uint8_t channel );

#endif // __ADC_MCU_H__

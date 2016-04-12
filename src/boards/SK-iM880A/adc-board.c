/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Board ADC driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Andreas Pella (IMST GmbH), Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "adc-board.h"

/*!
 * Calibration Data Bytes base address for medium density devices
 */
#define FACTORY_TSCALIB_BASE                        ( ( uint32_t )0x1FF80078 )
#define PDDADC_AVG_SLOPE                            1610 // 1.61 * 1000
#define PDDADC_OVERSAMPLE_FACTOR                    0x04

void AdcMcuInit( Adc_t *obj, PinNames adcInput )
{
    obj->Adc.Instance = ( ADC_TypeDef *)ADC1_BASE;
    GpioInit( &obj->AdcInput, adcInput, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void AdcMcuFormat( Adc_t *obj, AdcResolution AdcRes, AdcNumConversion AdcNumConv, AdcTriggerConv AdcTrig, AdcDataAlignement AdcDataAlig )
{
    ADC_HandleTypeDef *adc;

    if( AdcRes == ADC_12_BIT )
    {
        obj->Adc.Init.Resolution = ADC_RESOLUTION_12B;
    }
    else if( AdcRes == ADC_10_BIT )
    {
        obj->Adc.Init.Resolution = ADC_RESOLUTION_10B;
    }
    else if( AdcRes == ADC_8_BIT )
    {
        obj->Adc.Init.Resolution = ADC_RESOLUTION_8B;
    }
    else if( AdcRes == ADC_6_BIT )
    {
        obj->Adc.Init.Resolution = ADC_RESOLUTION_6B;
    }

    if( AdcNumConv == SINGLE_CONVERSION )
    {
        obj->Adc.Init.ContinuousConvMode = DISABLE;
    }
    else
    {
        obj->Adc.Init.ContinuousConvMode = ENABLE;
    }

    if( AdcTrig == CONVERT_MANUAL_TRIG )
    {
        obj->Adc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONVEDGE_NONE;
    }
    else if( AdcTrig == CONVERT_RISING_EDGE )
    {
        obj->Adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    }
    else if( AdcTrig == CONVERT_FALLING_EDGE )
    {
        obj->Adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
    }
    else
    {
        obj->Adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING;
    }

    if( AdcDataAlig == DATA_RIGHT_ALIGNED )
    {
        obj->Adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    }
    else
    {
        obj->Adc.Init.DataAlign = ADC_DATAALIGN_LEFT;
    }

        obj->Adc.Init.NbrOfConversion = 1;

        adc = &obj->Adc;
        HAL_ADC_Init( adc );
}

uint16_t AdcMcuRead( Adc_t *obj, uint8_t channel )
{
        ADC_HandleTypeDef *hadc;
        ADC_ChannelConfTypeDef adcConf;
        uint16_t adcData = 0;

        hadc = &obj->Adc;

            /* Enable HSI */
        __HAL_RCC_HSI_ENABLE();

        /* Wait till HSI is ready */
        while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
        {
        }

        __HAL_RCC_ADC1_CLK_ENABLE( );

        adcConf.Channel = channel;
        adcConf.Rank = ADC_REGULAR_RANK_1;
        adcConf.SamplingTime = ADC_SAMPLETIME_192CYCLES;

        HAL_ADC_ConfigChannel( hadc, &adcConf);

        /* Enable ADC1 */
        __HAL_ADC_ENABLE( hadc) ;

        /* Start ADC1 Software Conversion */
        HAL_ADC_Start( hadc);

        HAL_ADC_PollForConversion( hadc, HAL_MAX_DELAY );

        adcData = HAL_ADC_GetValue ( hadc);

        __HAL_ADC_DISABLE( hadc) ;

        if( ( adcConf.Channel == ADC_CHANNEL_TEMPSENSOR ) || ( adcConf.Channel == ADC_CHANNEL_VREFINT ) )
        {
                HAL_ADC_DeInit( hadc );
        }
        __HAL_RCC_ADC1_CLK_DISABLE( );

        /* Disable HSI */
        __HAL_RCC_HSI_DISABLE();

        return adcData;
}

/*!
 * \file      adc-board.c
 *
 * \brief     Target board ADC driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx.h"
#include "board-config.h"
#include "adc-board.h"

#define ADCCLK_ENABLE()                 __HAL_RCC_ADC1_CLK_ENABLE() ;
#define ADCCLK_DISABLE()                __HAL_RCC_ADC1_CLK_DISABLE() ;

ADC_HandleTypeDef AdcHandle;
bool AdcInitialized = false;


void AdcMcuInit( Adc_t *obj, PinNames adcInput )
{
    AdcHandle.Instance = ( ADC_TypeDef* )ADC1_BASE;

    __HAL_RCC_ADC1_CLK_ENABLE( );

    HAL_ADC_DeInit( &AdcHandle );

    if( adcInput != NC )
    {
        GpioInit( &obj->AdcInput, adcInput, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
}

void AdcMcuConfig( void )
{
    if( AdcInitialized == false )
    {
        AdcInitialized                       = true;

        AdcHandle.Instance                   = ADC1;

        AdcHandle.Init.OversamplingMode      = DISABLE;

        AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
        AdcHandle.Init.LowPowerAutoPowerOff  = DISABLE;
        AdcHandle.Init.LowPowerFrequencyMode = ENABLE;
        AdcHandle.Init.LowPowerAutoWait      = DISABLE;

        AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
        AdcHandle.Init.SamplingTime          = ADC_SAMPLETIME_160CYCLES_5;
        AdcHandle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;
        AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
        AdcHandle.Init.ContinuousConvMode    = DISABLE;
        AdcHandle.Init.DiscontinuousConvMode = DISABLE;
        AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
        AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
        AdcHandle.Init.DMAContinuousRequests = DISABLE;

        ADCCLK_ENABLE( );

        HAL_ADC_Init( &AdcHandle );
    }
}

uint16_t AdcMcuReadChannel( Adc_t *hadc, uint32_t channel )
{
    ADC_ChannelConfTypeDef adcConf;
    uint16_t adcData = 0;

    if( AdcInitialized == true )
    {
        /* wait the the Vrefint used by adc is set */
        while( __HAL_PWR_GET_FLAG( PWR_FLAG_VREFINTRDY ) == RESET )
        {
        };

        ADCCLK_ENABLE( );

        /*calibrate ADC if any calibraiton hardware*/
        HAL_ADCEx_Calibration_Start( &AdcHandle, ADC_SINGLE_ENDED );

        /* Deselects all channels*/
        adcConf.Channel = ADC_CHANNEL_MASK;
        adcConf.Rank = ADC_RANK_NONE;
        HAL_ADC_ConfigChannel( &AdcHandle, &adcConf );

        /* configure adc channel */
        adcConf.Channel = channel;
        adcConf.Rank = ADC_RANK_CHANNEL_NUMBER;
        HAL_ADC_ConfigChannel( &AdcHandle, &adcConf );

        /* Start the conversion process */
        HAL_ADC_Start( &AdcHandle );

        /* Wait for the end of conversion */
        HAL_ADC_PollForConversion( &AdcHandle, HAL_MAX_DELAY );

        /* Get the converted value of regular channel */
        adcData = HAL_ADC_GetValue( &AdcHandle );

        __HAL_ADC_DISABLE( &AdcHandle );

        ADCCLK_DISABLE( );
    }
    return adcData;
}

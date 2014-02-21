/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    ©2013 Semtech

Description: Board ADC driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "adc-board.h"

void AdcMcuInit( Adc_t *obj, PinNames adcInput )
{
    obj->Adc = ( ADC_TypeDef *)ADC1_BASE;

    ADC_DeInit( obj->Adc );   

    GpioInit( &obj->AdcInput, adcInput, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void AdcMcuFormat( Adc_t *obj, AdcResolution AdcRes, AdcNumConversion AdcNumConv, AdcTriggerConv AdcTrig, AdcDataAlignement AdcDataAlig )
{
    /* Enable The HSI (16Mhz) */
    RCC_HSICmd( ENABLE );

    /* Check that HSI oscillator is ready */
    while(RCC_GetFlagStatus( RCC_FLAG_HSIRDY ) == RESET );

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE );

    ADC_InitTypeDef ADC_InitStructure;

    ADC_StructInit( &ADC_InitStructure );

    if( AdcRes == ADC_12_BIT )
    {
        ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    }
    else if( AdcRes == ADC_10_BIT )
    {
        ADC_InitStructure.ADC_Resolution = ADC_Resolution_10b;
    }
    else if(AdcRes == ADC_8_BIT )
    {
        ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
    } 
    else if(AdcRes == ADC_6_BIT )
    {
        ADC_InitStructure.ADC_Resolution = ADC_Resolution_6b;
    } 

    ADC_InitStructure.ADC_ScanConvMode = DISABLE;

    if( AdcNumConv == SINGLE_CONVERSION )
    {
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    }
    else
    {
        ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    }

    if( AdcTrig == CONVERT_MANUAL_TRIG )
    {
        ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    }
    else if( AdcTrig == CONVERT_RISING_EDGE ) 
    {
        ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    }
    else if( AdcTrig == CONVERT_FALLING_EDGE ) 
    {
        ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Falling;
    }
    else 
    {
        ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_RisingFalling;
    }
        
    if( AdcDataAlig == DATA_RIGHT_ALIGNED )
    {
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    }
    else
    {
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
    }

    ADC_InitStructure.ADC_NbrOfConversion = 1;

    ADC_Init( ADC1, &ADC_InitStructure );

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, DISABLE );

    RCC_HSICmd( DISABLE );

}

uint16_t AdcMcuReadChannel( Adc_t *obj )
{
    uint16_t ADCdata = 0;

    /* Enable The HSI (16Mhz) */
    RCC_HSICmd( ENABLE );

    /* Check that HSI oscillator is ready */
    while( RCC_GetFlagStatus( RCC_FLAG_HSIRDY ) == RESET );

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE );


    /* ADC1 regular channel5 or channel1 configuration */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_3, 1, ADC_SampleTime_192Cycles );

    /* Define delay between ADC1 conversions */
    ADC_DelaySelectionConfig( ADC1, ADC_DelayLength_Freeze );

    /* Enable ADC1 Power Down during Delay */
    ADC_PowerDownCmd( ADC1, ADC_PowerDown_Idle_Delay, ENABLE );

    /* Enable ADC1 */
    ADC_Cmd( ADC1, ENABLE );

    /* Wait until ADC1 ON status */
    while( ADC_GetFlagStatus( ADC1, ADC_FLAG_ADONS ) == RESET )
    {
    }

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConv( ADC1 );

    /* Wait until ADC Channel 5 or 1 end of conversion */
    while( ADC_GetFlagStatus( ADC1, ADC_FLAG_EOC ) == RESET )
    {
    }

    ADCdata = ADC_GetConversionValue( ADC1 );

    ADC_Cmd( ADC1, DISABLE );
    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, DISABLE );

    RCC_HSICmd( DISABLE );
   
    return ADCdata;
}



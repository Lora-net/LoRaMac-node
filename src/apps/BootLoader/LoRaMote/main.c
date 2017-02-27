/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMote board USB DFU bootloader

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_flash.h"

extern PCD_HandleTypeDef hpcd;

USBD_HandleTypeDef USBD_Device;
pFunction JumpToApplication;
uint32_t JumpAddress;

/*
 * Board peripherals objects
 */
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

I2c_t I2c;

void SystemClockConfig( void );

static void DelayLoop( volatile uint32_t nCount )
{
    volatile uint32_t index = 0;
    for( index = ( 5000 * nCount ); index != 0; index-- )
    {
    }
}

int main( void )
{
    uint8_t regValue = 0;
    uint8_t status = 0;
    uint16_t offset = 0;

    /* STM32L1xx HAL library initialization:
         - Configure the Flash prefetch
         - Systick timer is configured by default as source of time base, but user
           can eventually implement his proper time base source (a general purpose
           timer for example or other time source), keeping in mind that Time base
           duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
           handled in milliseconds basis.
         - Set NVIC Group Priority to 4
         - Low Level Initialization
       */
    HAL_Init( );

    SystemClockConfig( );

    I2cInit( &I2c, I2C_SCL, I2C_SDA );

    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    // Init SAR
    SX9500Init( );
    DelayLoop( 100 );
    SX9500Write( SX9500_REG_IRQMSK, 0x10 );
    SX9500Write( SX9500_REG_IRQSRC, 0x10 );

    do
    {
        SX9500Read( SX9500_REG_IRQSRC, &status );
    }while( ( status & 0x10 ) == 0x00 ); // While compensation for CS0 is pending

    // Read 1st sensor offset
    SX9500Read( SX9500_REG_OFFSETMSB, ( uint8_t* )&regValue );
    offset = regValue << 8;
    SX9500Read( SX9500_REG_OFFSETLSB, ( uint8_t* )&regValue );
    offset |= regValue;

    if( offset < 2000 )
    { /* Test if user code is programmed starting from address 0x08007000 */
        if( ( ( *( volatile uint32_t* )USBD_DFU_APP_DEFAULT_ADD ) & 0x2FFE0000 ) == 0x20000000 )
        {
            /* Jump to user application */
            JumpAddress = *( volatile uint32_t* ) ( USBD_DFU_APP_DEFAULT_ADD + 4 );
            JumpToApplication = ( pFunction ) JumpAddress;

            /* Initialize user application's Stack Pointer */
            __set_MSP( *( volatile uint32_t* ) USBD_DFU_APP_DEFAULT_ADD );
            JumpToApplication( );
        }
    } /* Otherwise enters DFU mode to allow user to program his application */

    /* Init Device Library */
    USBD_Init( &USBD_Device, &DFU_Desc, 0 );

    /* Add Supported Class */
    USBD_RegisterClass( &USBD_Device, USBD_DFU_CLASS );

    /* Add DFU Media interface */
    USBD_DFU_RegisterMedia( &USBD_Device, &USBD_DFU_Flash_fops );

    /* Start Device Process */
    USBD_Start( &USBD_Device );

    /* Main loop */
    while( 1 )
    {
        GpioWrite( &Led1, 0 );
        GpioWrite( &Led2, 0 );
        GpioWrite( &Led3, 0 );
        DelayLoop( 500 );
        GpioWrite( &Led1, 1 );
        GpioWrite( &Led2, 1 );
        GpioWrite( &Led3, 1 );
        DelayLoop( 500 );
    }
}

/*!
 * System Clock Configuration
 */
void SystemClockConfig( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    /* Enable HSI Oscillator and Activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL6;
    RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLL_DIV3;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
    clocks dividers */
    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 );
}

void SysTick_Handler( void )
{
    HAL_IncTick( );
}

void USB_LP_IRQHandler( void )
{
    HAL_PCD_IRQHandler( &hpcd );
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif

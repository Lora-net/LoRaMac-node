#include "gpio.h"
#include "gpio-board.h"
#include "pinName-board.h"
#include "stm32l1xx.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_rcc_ex.h"

GPIO_InitTypeDef GPIO_InitStructure;

/// Spin delay
void delay(int count) {
  // volatile so that the compiler doesn't optimise it out
  volatile int i;

  for (i = 0; i < count; i++) {
  }
}

/// Main function.  Called by the startup code.
int main(void) {

  /* GPIOC Periph clock enable */
  /* RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInitStruct);
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_0;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.OType = GPIO_OType_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIOA->MODER = 0x55555555;   // set pin 0 and 1 as output
  GPIOA->OTYPER = 0x0;         // set both pins as output push/pull
  GPIOA->OSPEEDR = 0xFFFFFFFF; // set both pins as very high speed
  GPIOA->PUPDR = 0x55555555;   // set both pins to pull up
*/

  // using same method as system_stm32l1xx.c
  __IO uint32_t tmpreg = 0;
  FLASH->ACR |= FLASH_ACR_LATENCY;
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
  PWR->CR = PWR_CR_VOS_0;

  // wait for voltage regulator to be ready
  while ((PWR->CSR & PWR_CSR_VOSF) != RESET) {
  }
  RCC->AHBENR = 0x000080DF;
  tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_GPIODEN);
  GPIOA->MODER = 0xa8555555;
  GPIOA->OTYPER = 0x0;
  GPIOA->OSPEEDR = 0xFFFFFFFF;
  GPIOA->PUPDR = 0x64555555;

  (void)(tmpreg);

  for (;;) {
    // Turn on both LEDs
    /*GPIOA->BSRR = 0x03;

    // Around 1/4 of a second
    delay(2400000);

    // Turn off the first led
    GPIOA->BRR= 0x01;

    delay(2400000);

    // Turn off the second LED and the on the first
    GPIOA->BRR = 0x02;
    GPIOA->BSRR = 0x01;

    delay(2400000);

    // Turn off first
    GPIOA->BRR = 0x01;

    delay(2400000); */

    GPIOA->BSRR = 0xFFFF;
    delay(24000);
    GPIOA->BSRR = 0xFFFF0000;
    delay(24000);
  }
}

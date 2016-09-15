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

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  //RCC->AHBENR = 0x00008001;
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_8;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  // using same method as system_stm32l1xx.c
  /* __IO uint32_t tmpreg = 0;
  FLASH->ACR |= FLASH_ACR_LATENCY;
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
  PWR->CR = PWR_CR_VOS_0;

  // wait for voltage regulator to be ready
  while ((PWR->CSR & PWR_CSR_VOSF) != RESET) {
  }
  GPIOA->MODER = 0xa8555555;
  GPIOA->OTYPER = 0x0;
  GPIOA->OSPEEDR = 0xFFFFFFFF;
  GPIOA->PUPDR = 0x64555555;
*/

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

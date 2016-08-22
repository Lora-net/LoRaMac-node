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
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInitStruct);

  GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  // GPIO_InitStructure.OType = GPIO_OType_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  for (;;) {
    // Turn on both LEDs
    GPIOC->BSRR = 0x0300;

    // Around 1/4 of a second
    delay(2400000);

    // Turn off the first led
    GPIOC->BRR = 0x0100;

    delay(2400000);

    // Turn off the second LED and the on the first
    GPIOC->BRR = 0x0200;
    GPIOC->BSRR = 0x0100;

    delay(2400000);

    // Turn off first
    GPIOC->BRR = 0x0100;

    delay(2400000);
  }
}

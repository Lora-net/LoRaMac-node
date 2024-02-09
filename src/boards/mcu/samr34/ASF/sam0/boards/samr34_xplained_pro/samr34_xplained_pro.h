/**
 * \file
 *
 * \brief SAM R34 Xplained Pro board definition
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
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
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
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

#ifndef SAMR34_XPLAINED_PRO_H_INCLUDED
#define SAMR34_XPLAINED_PRO_H_INCLUDED

#include <conf_board.h>
#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup group_common_boards
 * \defgroup samr34_xplained_pro_group SAM R34 Xplained Pro board
 *
 * @{
 */

void system_board_init(void);

/**
 * \defgroup samr34xplained_pro_features_group Features
 *
 * Symbols that describe features and capabilities of the board.
 *
 * @{
 */

/** Name string macro */
#define BOARD_NAME                "SAMR34_XPLAINED_PRO"

/** \name Resonator definitions
 *  @{ */
#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    0 /* Not Mounted */
#define BOARD_FREQ_MAINCK_BYPASS  0 /* Not Mounted */
#define BOARD_MCK                 CHIP_FREQ_CPU_MAX
#define BOARD_OSC_STARTUP_US      15625
/** @} */

/** \name LED0 definitions
 *  @{ */
#define LED0_PIN                  PIN_PA19
#define LED0_ACTIVE               false
#define LED0_INACTIVE             !LED0_ACTIVE
/** @} */

/** \name LED1 definitions
 *  @{ */
#define LED1_PIN                  PIN_PA18
#define LED1_ACTIVE               false
#define LED1_INACTIVE             !LED1_ACTIVE
/** @} */

/** \name SW0 definitions
 *  @{ */
#define SW0_PIN                   PIN_PA28
#define SW0_ACTIVE                false
#define SW0_INACTIVE              !SW0_ACTIVE
#define SW0_EIC_PIN               PIN_PA28
#define SW0_EIC_MUX               MUX_PA28A_EIC_EXTINT8
#define SW0_EIC_PINMUX            PINMUX_PA28A_EIC_EXTINT8
#define SW0_EIC_LINE              8
/** @} */

/** \name RF SWITCH definitions
 *  @{ */
#define RF_SWITCH_PIN				PIN_PA13
#define RF_SWITCH_ACTIVE             true
#define RF_SWITCH_INACTIVE	        !RF_SWITCH_ACTIVE
/** @} */

/** \name TCXO PWR Pin definitions
 *  @{ */
#define TCXO_PWR_PIN				PIN_PA09
#define TCXO_PWR_ACTIVE             true
#define TCXO_PWR_INACTIVE	        !TCXO_PWR_ACTIVE
/** @} */


/**
 * \name LED #0 definitions
 *
 * Wrapper macros for LED0, to ensure common naming across all Xplained Pro
 * boards.
 *
 *  @{ */
#define LED_0_NAME                "LED0 (yellow)"
#define LED_0_PIN                 LED0_PIN
#define LED_0_ACTIVE              LED0_ACTIVE
#define LED_0_INACTIVE            LED0_INACTIVE
#define LED0_GPIO                 LED0_PIN
#define LED0                      LED0_PIN

#define LED_0_PWM3CTRL_MODULE     TCC0
#define LED_0_PWM3CTRL_CHANNEL    3
#define LED_0_PWM3CTRL_OUTPUT     3
#define LED_0_PWM3CTRL_PIN        PIN_PA19F_TCC0_WO3
#define LED_0_PWM3CTRL_MUX        MUX_PA19F_TCC0_WO3
#define LED_0_PWM3CTRL_PINMUX     PINMUX_PA19F_TCC0_WO3
/** @} */

/**
 * \name LED #1 definitions
 *
 * Wrapper macros for LED1, to ensure common naming across all Xplained Pro
 * boards.
 *
 *  @{ */
#define LED_1_NAME                "LED1 (Green)"
#define LED_1_PIN                 LED1_PIN
#define LED_1_ACTIVE              LED1_ACTIVE
#define LED_1_INACTIVE            LED1_INACTIVE
#define LED1_GPIO                 LED1_PIN
#define LED1                      LED1_PIN

#define LED_1_PWM2CTRL_MODULE     TCC0
#define LED_1_PWM2CTRL_CHANNEL    2
#define LED_1_PWM2CTRL_OUTPUT     2
#define LED_1_PWM2CTRL_PIN        PINMUX_PA18F_TCC0_WO2
#define LED_1_PWM2CTRL_MUX        MUX_PA18F_TCC0_WO2
#define LED_1_PWM2CTRL_PINMUX     PINMUX_PA18F_TCC0_WO2
/** @} */

/** Number of on-board LEDs */
#define LED_COUNT                 2


/**
 * \name Button #0 definitions
 *
 * Wrapper macros for SW0, to ensure common naming across all Xplained Pro
 * boards.
 *
 *  @{ */
#define BUTTON_0_NAME             "SW0"
#define BUTTON_0_PIN              SW0_PIN
#define BUTTON_0_ACTIVE           SW0_ACTIVE
#define BUTTON_0_INACTIVE         SW0_INACTIVE
#define BUTTON_0_EIC_PIN          SW0_EIC_PIN
#define BUTTON_0_EIC_MUX          SW0_EIC_MUX
#define BUTTON_0_EIC_PINMUX       SW0_EIC_PINMUX
#define BUTTON_0_EIC_LINE         SW0_EIC_LINE
/** @} */

/** Number of on-board buttons */
#define BUTTON_COUNT 1

/** \name Extension header #1 pin definitions
 *  @{
 */
#define EXT1_PIN_3                PIN_PA06
#define EXT1_PIN_4                PIN_PA07
#define EXT1_PIN_5                PIN_PA08
#define EXT1_PIN_6                PIN_PA28
#define EXT1_PIN_7                PIN_PA18
#define EXT1_PIN_8                PIN_PA19
#define EXT1_PIN_9                PIN_PA22
#define EXT1_PIN_10               PIN_PA15
#define EXT1_PIN_11               PIN_PA16
#define EXT1_PIN_12               PIN_PA17
#define EXT1_PIN_13               PIN_PA05
#define EXT1_PIN_14               PIN_PA04
#define EXT1_PIN_15               PIN_PA23
#define EXT1_PIN_16               PIN_PB22
#define EXT1_PIN_17               PIN_PB02
#define EXT1_PIN_18               PIN_PB23
/** @} */

/** \name Extension header #1 pin definitions by function
 *  @{
 */
#define EXT1_PIN_ADC_0            EXT1_PIN_3
#define EXT1_PIN_ADC_1            EXT1_PIN_4
#define EXT1_PIN_GPIO_0           EXT1_PIN_5
#define EXT1_PIN_GPIO_1           EXT1_PIN_6
#define EXT1_PIN_PWM_0            EXT1_PIN_7
#define EXT1_PIN_PWM_1            EXT1_PIN_8
#define EXT1_PIN_IRQ              EXT1_PIN_9
#define EXT1_PIN_I2C_SDA          EXT1_PIN_11
#define EXT1_PIN_I2C_SCL          EXT1_PIN_12
#define EXT1_PIN_UART_RX          EXT1_PIN_13
#define EXT1_PIN_UART_TX          EXT1_PIN_14
#define EXT1_PIN_SPI_SS_1         EXT1_PIN_10
#define EXT1_PIN_SPI_SS_0         EXT1_PIN_15
#define EXT1_PIN_SPI_MOSI         EXT1_PIN_16
#define EXT1_PIN_SPI_MISO         EXT1_PIN_17
#define EXT1_PIN_SPI_SCK          EXT1_PIN_18
/** @} */

/** \name Extension header #1 ADC definitions
 *  @{
 */
#define EXT1_ADC_MODULE           ADC
#define EXT1_ADC_6_CHANNEL        6
#define EXT1_ADC_6_PIN            PIN_PA06B_ADC_AIN6
#define EXT1_ADC_6_MUX            MUX_PA06B_ADC_AIN6
#define EXT1_ADC_6_PINMUX         PINMUX_PA06B_ADC_AIN6
#define EXT1_ADC_7_CHANNEL        7
#define EXT1_ADC_7_PIN            PIN_PA07B_ADC_AIN7
#define EXT1_ADC_7_MUX            MUX_PA07B_ADC_AIN7
#define EXT1_ADC_7_PINMUX         PINMUX_PA07B_ADC_AIN7
/** @} */

/** \name Extension header #1 PWM definitions
 *  @{
 */
#define EXT1_PWM_MODULE           TCC0
#define EXT1_PWM_0_CHANNEL        2
#define EXT1_PWM_0_PIN            PIN_PA18F_TCC0_WO2
#define EXT1_PWM_0_MUX            MUX_PA18F_TCC0_WO2
#define EXT1_PWM_0_PINMUX         PINMUX_PA18F_TCC0_WO2
#define EXT1_PWM_1_CHANNEL        3
#define EXT1_PWM_1_PIN            PIN_PA19F_TCC0_WO3
#define EXT1_PWM_1_MUX            MUX_PA19F_TCC0_WO3
#define EXT1_PWM_1_PINMUX         PINMUX_PA19F_TCC0_WO3
/** @} */

/** \name Extension header #1 IRQ/External interrupt definitions
 *  @{
 */
#define EXT1_IRQ_MODULE           EIC
#define EXT1_IRQ_INPUT            6
#define EXT1_IRQ_PIN              PIN_PA22A_EIC_EXTINT6
#define EXT1_IRQ_MUX              MUX_PA22A_EIC_EXTINT6
#define EXT1_IRQ_PINMUX           PINMUX_PA22A_EIC_EXTINT6
/** @} */

/** \name Extension header #1 I2C definitions
 *  @{
 */
#define EXT1_I2C_MODULE              SERCOM1
#define EXT1_I2C_SERCOM_PINMUX_PAD0  PINMUX_PA16C_SERCOM1_PAD0
#define EXT1_I2C_SERCOM_PINMUX_PAD1  PINMUX_PA17C_SERCOM1_PAD1
#define EXT1_I2C_SERCOM_DMAC_ID_TX   SERCOM1_DMAC_ID_TX
#define EXT1_I2C_SERCOM_DMAC_ID_RX   SERCOM1_DMAC_ID_RX
/** @} */

/** \name Extension header #1 UART definitions
 *  @{
 */
#define EXT1_UART_MODULE              SERCOM0
#define EXT1_UART_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1
#define EXT1_UART_SERCOM_PINMUX_PAD0  PINMUX_PA04D_SERCOM0_PAD0
#define EXT1_UART_SERCOM_PINMUX_PAD1  PINMUX_PA05D_SERCOM0_PAD1
#define EXT1_UART_SERCOM_PINMUX_PAD2  PINMUX_UNUSED
#define EXT1_UART_SERCOM_PINMUX_PAD3  PINMUX_UNUSED
#define EXT1_UART_SERCOM_DMAC_ID_TX   SERCOM0_DMAC_ID_TX
#define EXT1_UART_SERCOM_DMAC_ID_RX   SERCOM0_DMAC_ID_RX

#define EXT1_UART_SERCOM_PIN_PAD0     PIN_PA04D_SERCOM0_PAD0
#define EXT1_UART_SERCOM_PIN_PAD1     PIN_PA05D_SERCOM0_PAD1
/** @} */

/** \name Extension header #1 SPI definitions
 *  @{
 */
#define EXT1_SPI_MODULE              SERCOM5
#define EXT1_SPI_SERCOM_MUX_SETTING  SPI_SIGNAL_MUX_SETTING_E
#define EXT1_SPI_SERCOM_PINMUX_PAD0  PINMUX_PB02D_SERCOM5_PAD0
#define EXT1_SPI_SERCOM_PINMUX_PAD1  PINMUX_PA23D_SERCOM5_PAD1
#define EXT1_SPI_SERCOM_PINMUX_PAD2  PINMUX_PB22D_SERCOM5_PAD2
#define EXT1_SPI_SERCOM_PINMUX_PAD3  PINMUX_PB23D_SERCOM5_PAD3
#define EXT1_SPI_SERCOM_DMAC_ID_TX   SERCOM5_DMAC_ID_TX
#define EXT1_SPI_SERCOM_DMAC_ID_RX   SERCOM5_DMAC_ID_RX
/** @} */

/** \name Extension header #3 pin definitions
 *  @{
 */
#define EXT3_PIN_5                PIN_PA27
#define EXT3_PIN_10               PIN_PA08
#define EXT3_PIN_11               PIN_PA16
#define EXT3_PIN_12               PIN_PA17
#define EXT3_PIN_15               PIN_PA14
#define EXT3_PIN_16               PIN_PB22
#define EXT3_PIN_17               PIN_PB02
#define EXT3_PIN_18               PIN_PB23
/** @} */

/** \name Extension header #3 pin definitions by function
 *  @{
 */
#define EXT3_PIN_GPIO_1           EXT3_PIN_5
#define EXT3_PIN_I2C_SDA          EXT3_PIN_11
#define EXT3_PIN_I2C_SCL          EXT3_PIN_12
#define EXT3_PIN_SPI_SS_1         EXT3_PIN_10
#define EXT3_PIN_SPI_SS_0         EXT3_PIN_15
#define EXT3_PIN_SPI_MOSI         EXT3_PIN_16
#define EXT3_PIN_SPI_MISO         EXT3_PIN_17
#define EXT3_PIN_SPI_SCK          EXT3_PIN_18
/** @} */


/** \name Extension header #3 IRQ/External interrupt definitions
 *  @{
 */
#define EXT3_IRQ_MODULE           EIC
#define EXT3_IRQ_INPUT            0
#define EXT3_IRQ_PIN              PIN_PA16A_EIC_EXTINT0
#define EXT3_IRQ_MUX              MUX_PA16A_EIC_EXTINT0
#define EXT3_IRQ_PINMUX           PINMUX_PA16A_EIC_EXTINT0
/** @} */

/** \name Extension header #3 I2C definitions
 *  @{
 */
#define EXT3_I2C_MODULE              SERCOM1
#define EXT3_I2C_SERCOM_PINMUX_PAD0  PINMUX_PA16C_SERCOM1_PAD0
#define EXT3_I2C_SERCOM_PINMUX_PAD1  PINMUX_PA17C_SERCOM1_PAD1
#define EXT3_I2C_SERCOM_DMAC_ID_TX   SERCOM1_DMAC_ID_TX
#define EXT3_I2C_SERCOM_DMAC_ID_RX   SERCOM1_DMAC_ID_RX
/** @} */

/** \name Extension header #3 UART definitions
 *  @{
 */
#define EXT3_UART_MODULE              SERCOM1
#define EXT3_UART_SERCOM_MUX_SETTING  USART_RX_3_TX_2_XCK_3
#define EXT3_UART_SERCOM_PINMUX_PAD0  PINMUX_UNUSED
#define EXT3_UART_SERCOM_PINMUX_PAD1  PINMUX_UNUSED
#define EXT3_UART_SERCOM_PINMUX_PAD2  PINMUX_PA18C_SERCOM1_PAD2
#define EXT3_UART_SERCOM_PINMUX_PAD3  PINMUX_PA19C_SERCOM1_PAD3
#define EXT3_UART_SERCOM_DMAC_ID_TX   SERCOM1_DMAC_ID_TX
#define EXT3_UART_SERCOM_DMAC_ID_RX   SERCOM1_DMAC_ID_RX

/** @} */

/** \name Extension header #3 SPI definitions
 *  @{
 */
#define EXT3_SPI_MODULE              SERCOM5
#define EXT3_SPI_SERCOM_MUX_SETTING  SPI_SIGNAL_MUX_SETTING_E
#define EXT3_SPI_SERCOM_PINMUX_PAD0  PINMUX_PB02D_SERCOM5_PAD0
#define EXT3_SPI_SERCOM_PINMUX_PAD1  PINMUX_UNUSED
#define EXT3_SPI_SERCOM_PINMUX_PAD2  PINMUX_PB22D_SERCOM5_PAD2
#define EXT3_SPI_SERCOM_PINMUX_PAD3  PINMUX_PB23D_SERCOM5_PAD3
#define EXT3_SPI_SERCOM_DMAC_ID_TX   SERCOM5_DMAC_ID_TX
#define EXT3_SPI_SERCOM_DMAC_ID_RX   SERCOM5_DMAC_ID_RX
/** @} */

/** \name Embedded debugger I2C interface definitions
 * @{
 */
#define EDBG_I2C_MODULE              SERCOM1
#define EDBG_I2C_SERCOM_PINMUX_PAD0  PINMUX_PA16C_SERCOM1_PAD0
#define EDBG_I2C_SERCOM_PINMUX_PAD1  PINMUX_PA17C_SERCOM1_PAD1
#define EDBG_I2C_SERCOM_DMAC_ID_TX   SERCOM1_DMAC_ID_TX
#define EDBG_I2C_SERCOM_DMAC_ID_RX   SERCOM1_DMAC_ID_RX
/** @} */

/** \name Embedded debugger SPI interface definitions
 * @{
 */
#define EDBG_SPI_MODULE              SERCOM5
#define EDBG_SPI_SERCOM_MUX_SETTING  SPI_SIGNAL_MUX_SETTING_E
#define EDBG_SPI_SERCOM_PINMUX_PAD0  PINMUX_PB02D_SERCOM5_PAD0
#define EDBG_SPI_SERCOM_PINMUX_PAD1  PINMUX_UNUSED
#define EDBG_SPI_SERCOM_PINMUX_PAD2  PINMUX_PB22D_SERCOM5_PAD2
#define EDBG_SPI_SERCOM_PINMUX_PAD3  PINMUX_PB23D_SERCOM5_PAD3
#define EDBG_SPI_SERCOM_DMAC_ID_TX   SERCOM5_DMAC_ID_TX
#define EDBG_SPI_SERCOM_DMAC_ID_RX   SERCOM5_DMAC_ID_RX
/** @} */

/** \name Embedded debugger CDC Gateway USART interface definitions
 * @{
 */
#define EDBG_CDC_MODULE              SERCOM0
#define EDBG_CDC_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1
#define EDBG_CDC_SERCOM_PINMUX_PAD0  PINMUX_PA04D_SERCOM0_PAD0
#define EDBG_CDC_SERCOM_PINMUX_PAD1  PINMUX_PA05D_SERCOM0_PAD1
#define EDBG_CDC_SERCOM_PINMUX_PAD2  PINMUX_UNUSED
#define EDBG_CDC_SERCOM_PINMUX_PAD3  PINMUX_UNUSED
#define EDBG_CDC_SERCOM_DMAC_ID_TX   SERCOM0_DMAC_ID_TX
#define EDBG_CDC_SERCOM_DMAC_ID_RX   SERCOM0_DMAC_ID_RX
/** @} */

/** \name USB definitions
 * @{
 */
#define USB_ID
#define USB_TARGET_DP_PIN            PIN_PA25G_USB_DP
#define USB_TARGET_DP_MUX            MUX_PA25G_USB_DP
#define USB_TARGET_DP_PINMUX         PINMUX_PA25G_USB_DP
#define USB_TARGET_DM_PIN            PIN_PA24G_USB_DM
#define USB_TARGET_DM_MUX            MUX_PA24G_USB_DM
#define USB_TARGET_DM_PINMUX         PINMUX_PA24G_USB_DM
#define USB_VBUS_PIN                 PIN_PA07
#define USB_VBUS_EIC_LINE            7
#define USB_VBUS_EIC_MUX             MUX_PA07A_EIC_EXTINT7
#define USB_VBUS_EIC_PINMUX          PINMUX_PA07A_EIC_EXTINT7
#define USB_ID_PIN                   PIN_PA15
#define USB_ID_EIC_LINE              15
#define USB_ID_EIC_MUX               MUX_PA15A_EIC_EXTINT15
#define USB_ID_EIC_PINMUX            PINMUX_PA15A_EIC_EXTINT15

#define SX_RF_SPI                  SERCOM4
#define SX_RF_RESET_PIN            PIN_PB15
#define SX_RF_SPI_CS               PIN_PB31
#define SX_RF_SPI_MOSI             PIN_PB30
#define SX_RF_SPI_MISO             PIN_PC19
#define SX_RF_SPI_SCK              PIN_PC18

#define SX_RF_SPI_SERCOM_MUX_SETTING   SPI_SIGNAL_MUX_SETTING_E
#define SX_RF_SPI_SERCOM_PINMUX_PAD0   PINMUX_PC19F_SERCOM4_PAD0
#define SX_RF_SPI_SERCOM_PINMUX_PAD1   PINMUX_UNUSED
#define SX_RF_SPI_SERCOM_PINMUX_PAD2   PINMUX_PB30F_SERCOM4_PAD2
#define SX_RF_SPI_SERCOM_PINMUX_PAD3   PINMUX_PC18F_SERCOM4_PAD3

#define DIO0_PIN                   PIN_PB16
#define DIO0_ACTIVE                true
#define DIO0_INACTIVE              !DIO0_ACTIVE
#define DIO0_EIC_PIN               PIN_PB16A_EIC_EXTINT0
#define DIO0_EIC_MUX               MUX_PB16A_EIC_EXTINT0
#define DIO0_EIC_PINMUX            PINMUX_PB16A_EIC_EXTINT0
#define DIO0_EIC_LINE              0 

#define DIO1_PIN                   PIN_PA11
#define DIO1_ACTIVE                true
#define DIO1_INACTIVE              !DIO1_ACTIVE
#define DIO1_EIC_PIN               PIN_PA11A_EIC_EXTINT11
#define DIO1_EIC_MUX               MUX_PA11A_EIC_EXTINT11
#define DIO1_EIC_PINMUX            PINMUX_PA11A_EIC_EXTINT11
#define DIO1_EIC_LINE              11

#define DIO2_PIN                   PIN_PA12
#define DIO2_ACTIVE                true
#define DIO2_INACTIVE              !DIO2_ACTIVE
#define DIO2_EIC_PIN               PIN_PA12A_EIC_EXTINT12
#define DIO2_EIC_MUX               MUX_PA12A_EIC_EXTINT12
#define DIO2_EIC_PINMUX            PINMUX_PA12A_EIC_EXTINT12
#define DIO2_EIC_LINE              12

#define DIO3_PIN                   PIN_PB17
#define DIO3_ACTIVE                true
#define DIO3_INACTIVE              !DIO3_ACTIVE
#define DIO3_EIC_PIN               PIN_PB17A_EIC_EXTINT1
#define DIO3_EIC_MUX               MUX_PB17A_EIC_EXTINT1
#define DIO3_EIC_PINMUX            PINMUX_PB17A_EIC_EXTINT1
#define DIO3_EIC_LINE              1

#define DIO4_PIN                   PIN_PA10
#define DIO4_ACTIVE                true
#define DIO4_INACTIVE              !DIO4_ACTIVE
#define DIO4_EIC_PIN               PIN_PA10A_EIC_EXTINT10
#define DIO4_EIC_MUX               MUX_PA10A_EIC_EXTINT10
#define DIO4_EIC_PINMUX            PINMUX_PA10A_EIC_EXTINT10
#define DIO4_EIC_LINE              10

#define DIO5_PIN                   PIN_PB00
#define DIO5_ACTIVE                true
#define DIO5_INACTIVE              !DIO5_ACTIVE
#define DIO5_EIC_PIN               PIN_PB00A_EIC_EXTINT0
#define DIO5_EIC_MUX               MUX_PB00A_EIC_EXTINT0
#define DIO5_EIC_PINMUX            PINMUX_PB00A_EIC_EXTINT0
#define DIO5_EIC_LINE              0

#define SX_RF_RESET_HIGH		   true
#define SX_RF_RESET_LOW		       !SX_RF_RESET_HIGH

/** @} */

/**
 * \brief Turns off the specified LEDs.
 *
 * \param led_gpio LED to turn off (LEDx_GPIO).
 *
 * \note The pins of the specified LEDs are set to GPIO output mode.
 */
#define LED_Off(led_gpio)     port_pin_set_output_level(led_gpio,true)

/**
 * \brief Turns on the specified LEDs.
 *
 * \param led_gpio LED to turn on (LEDx_GPIO).
 *
 * \note The pins of the specified LEDs are set to GPIO output mode.
 */
#define LED_On(led_gpio)      port_pin_set_output_level(led_gpio,false)

/**
 * \brief Toggles the specified LEDs.
 *
 * \param led_gpio LED to toggle (LEDx_GPIO).
 *
 * \note The pins of the specified LEDs are set to GPIO output mode.
 */
#define LED_Toggle(led_gpio)  port_pin_toggle_output_level(led_gpio)

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* SAMR34_XPLAINED_PRO_H_INCLUDED */

/* Auto-generated config file hpl_gclk_config.h */
#ifndef HPL_GCLK_CONFIG_H
#define HPL_GCLK_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <e> Generic clock generator 0 configuration
// <i> Indicates whether generic clock 0 configuration is enabled or not
// <id> enable_gclk_gen_0
#ifndef CONF_GCLK_GENERATOR_0_CONFIG
#define CONF_GCLK_GENERATOR_0_CONFIG 1
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 0 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 0
// <id> gclk_gen_0_oscillator
#ifndef CONF_GCLK_GEN_0_SOURCE
#define CONF_GCLK_GEN_0_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_0_runstdby
#ifndef CONF_GCLK_GEN_0_RUNSTDBY
#define CONF_GCLK_GEN_0_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_0_div_sel
#ifndef CONF_GCLK_GEN_0_DIVSEL
#define CONF_GCLK_GEN_0_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_0_oe
#ifndef CONF_GCLK_GEN_0_OE
#define CONF_GCLK_GEN_0_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_0_oov
#ifndef CONF_GCLK_GEN_0_OOV
#define CONF_GCLK_GEN_0_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_0_idc
#ifndef CONF_GCLK_GEN_0_IDC
#define CONF_GCLK_GEN_0_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_0_enable
#ifndef CONF_GCLK_GEN_0_GENEN
#define CONF_GCLK_GEN_0_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 0 division <0x0000-0xFFFF>
// <id> gclk_gen_0_div
#ifndef CONF_GCLK_GEN_0_DIV
#define CONF_GCLK_GEN_0_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 1 configuration
// <i> Indicates whether generic clock 1 configuration is enabled or not
// <id> enable_gclk_gen_1
#ifndef CONF_GCLK_GENERATOR_1_CONFIG
#define CONF_GCLK_GENERATOR_1_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 1 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 1
// <id> gclk_gen_1_oscillator
#ifndef CONF_GCLK_GEN_1_SOURCE
#define CONF_GCLK_GEN_1_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_1_runstdby
#ifndef CONF_GCLK_GEN_1_RUNSTDBY
#define CONF_GCLK_GEN_1_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_1_div_sel
#ifndef CONF_GCLK_GEN_1_DIVSEL
#define CONF_GCLK_GEN_1_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_1_oe
#ifndef CONF_GCLK_GEN_1_OE
#define CONF_GCLK_GEN_1_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_1_oov
#ifndef CONF_GCLK_GEN_1_OOV
#define CONF_GCLK_GEN_1_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_1_idc
#ifndef CONF_GCLK_GEN_1_IDC
#define CONF_GCLK_GEN_1_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_1_enable
#ifndef CONF_GCLK_GEN_1_GENEN
#define CONF_GCLK_GEN_1_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 1 division <0x0000-0xFFFF>
// <id> gclk_gen_1_div
#ifndef CONF_GCLK_GEN_1_DIV
#define CONF_GCLK_GEN_1_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 2 configuration
// <i> Indicates whether generic clock 2 configuration is enabled or not
// <id> enable_gclk_gen_2
#ifndef CONF_GCLK_GENERATOR_2_CONFIG
#define CONF_GCLK_GENERATOR_2_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 2 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 2
// <id> gclk_gen_2_oscillator
#ifndef CONF_GCLK_GEN_2_SOURCE
#define CONF_GCLK_GEN_2_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_2_runstdby
#ifndef CONF_GCLK_GEN_2_RUNSTDBY
#define CONF_GCLK_GEN_2_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_2_div_sel
#ifndef CONF_GCLK_GEN_2_DIVSEL
#define CONF_GCLK_GEN_2_DIVSEL 1
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_2_oe
#ifndef CONF_GCLK_GEN_2_OE
#define CONF_GCLK_GEN_2_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_2_oov
#ifndef CONF_GCLK_GEN_2_OOV
#define CONF_GCLK_GEN_2_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_2_idc
#ifndef CONF_GCLK_GEN_2_IDC
#define CONF_GCLK_GEN_2_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_2_enable
#ifndef CONF_GCLK_GEN_2_GENEN
#define CONF_GCLK_GEN_2_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 2 division <0x0000-0xFFFF>
// <id> gclk_gen_2_div
#ifndef CONF_GCLK_GEN_2_DIV
#define CONF_GCLK_GEN_2_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 3 configuration
// <i> Indicates whether generic clock 3 configuration is enabled or not
// <id> enable_gclk_gen_3
#ifndef CONF_GCLK_GENERATOR_3_CONFIG
#define CONF_GCLK_GENERATOR_3_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 3 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 3
// <id> gclk_gen_3_oscillator
#ifndef CONF_GCLK_GEN_3_SOURCE
#define CONF_GCLK_GEN_3_SOURCE GCLK_GENCTRL_SRC_OSCULP32K
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_3_runstdby
#ifndef CONF_GCLK_GEN_3_RUNSTDBY
#define CONF_GCLK_GEN_3_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_3_div_sel
#ifndef CONF_GCLK_GEN_3_DIVSEL
#define CONF_GCLK_GEN_3_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_3_oe
#ifndef CONF_GCLK_GEN_3_OE
#define CONF_GCLK_GEN_3_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_3_oov
#ifndef CONF_GCLK_GEN_3_OOV
#define CONF_GCLK_GEN_3_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_3_idc
#ifndef CONF_GCLK_GEN_3_IDC
#define CONF_GCLK_GEN_3_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_3_enable
#ifndef CONF_GCLK_GEN_3_GENEN
#define CONF_GCLK_GEN_3_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 3 division <0x0000-0xFFFF>
// <id> gclk_gen_3_div
#ifndef CONF_GCLK_GEN_3_DIV
#define CONF_GCLK_GEN_3_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 4 configuration
// <i> Indicates whether generic clock 4 configuration is enabled or not
// <id> enable_gclk_gen_4
#ifndef CONF_GCLK_GENERATOR_4_CONFIG
#define CONF_GCLK_GENERATOR_4_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 4 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 4
// <id> gclk_gen_4_oscillator
#ifndef CONF_GCLK_GEN_4_SOURCE
#define CONF_GCLK_GEN_4_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_4_runstdby
#ifndef CONF_GCLK_GEN_4_RUNSTDBY
#define CONF_GCLK_GEN_4_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_4_div_sel
#ifndef CONF_GCLK_GEN_4_DIVSEL
#define CONF_GCLK_GEN_4_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_4_oe
#ifndef CONF_GCLK_GEN_4_OE
#define CONF_GCLK_GEN_4_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_4_oov
#ifndef CONF_GCLK_GEN_4_OOV
#define CONF_GCLK_GEN_4_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_4_idc
#ifndef CONF_GCLK_GEN_4_IDC
#define CONF_GCLK_GEN_4_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_4_enable
#ifndef CONF_GCLK_GEN_4_GENEN
#define CONF_GCLK_GEN_4_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 4 division <0x0000-0xFFFF>
// <id> gclk_gen_4_div
#ifndef CONF_GCLK_GEN_4_DIV
#define CONF_GCLK_GEN_4_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 5 configuration
// <i> Indicates whether generic clock 5 configuration is enabled or not
// <id> enable_gclk_gen_5
#ifndef CONF_GCLK_GENERATOR_5_CONFIG
#define CONF_GCLK_GENERATOR_5_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 5 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 5
// <id> gclk_gen_5_oscillator
#ifndef CONF_GCLK_GEN_5_SOURCE
#define CONF_GCLK_GEN_5_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_5_runstdby
#ifndef CONF_GCLK_GEN_5_RUNSTDBY
#define CONF_GCLK_GEN_5_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_5_div_sel
#ifndef CONF_GCLK_GEN_5_DIVSEL
#define CONF_GCLK_GEN_5_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_5_oe
#ifndef CONF_GCLK_GEN_5_OE
#define CONF_GCLK_GEN_5_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_5_oov
#ifndef CONF_GCLK_GEN_5_OOV
#define CONF_GCLK_GEN_5_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_5_idc
#ifndef CONF_GCLK_GEN_5_IDC
#define CONF_GCLK_GEN_5_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_5_enable
#ifndef CONF_GCLK_GEN_5_GENEN
#define CONF_GCLK_GEN_5_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 5 division <0x0000-0xFFFF>
// <id> gclk_gen_5_div
#ifndef CONF_GCLK_GEN_5_DIV
#define CONF_GCLK_GEN_5_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 6 configuration
// <i> Indicates whether generic clock 6 configuration is enabled or not
// <id> enable_gclk_gen_6
#ifndef CONF_GCLK_GENERATOR_6_CONFIG
#define CONF_GCLK_GENERATOR_6_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 6 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 6
// <id> gclk_gen_6_oscillator
#ifndef CONF_GCLK_GEN_6_SOURCE
#define CONF_GCLK_GEN_6_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_6_runstdby
#ifndef CONF_GCLK_GEN_6_RUNSTDBY
#define CONF_GCLK_GEN_6_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_6_div_sel
#ifndef CONF_GCLK_GEN_6_DIVSEL
#define CONF_GCLK_GEN_6_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_6_oe
#ifndef CONF_GCLK_GEN_6_OE
#define CONF_GCLK_GEN_6_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_6_oov
#ifndef CONF_GCLK_GEN_6_OOV
#define CONF_GCLK_GEN_6_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_6_idc
#ifndef CONF_GCLK_GEN_6_IDC
#define CONF_GCLK_GEN_6_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_6_enable
#ifndef CONF_GCLK_GEN_6_GENEN
#define CONF_GCLK_GEN_6_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 6 division <0x0000-0xFFFF>
// <id> gclk_gen_6_div
#ifndef CONF_GCLK_GEN_6_DIV
#define CONF_GCLK_GEN_6_DIV 1
#endif
// </h>
// </e>

// <e> Generic clock generator 7 configuration
// <i> Indicates whether generic clock 7 configuration is enabled or not
// <id> enable_gclk_gen_7
#ifndef CONF_GCLK_GENERATOR_7_CONFIG
#define CONF_GCLK_GENERATOR_7_CONFIG 0
#endif

// <h> Generic Clock Generator Control
// <y> Generic clock generator 7 source// <GCLK_GENCTRL_SRC_XOSC"> External Crystal Oscillator 0.4-32MHz (XOSC)
// <GCLK_GENCTRL_SRC_GCLKIN"> Generic clock generator input pad
// <GCLK_GENCTRL_SRC_GCLKGEN1"> Generic clock generator 1
// <GCLK_GENCTRL_SRC_OSCULP32K"> 32kHz Ultra Low Power Internal Oscillator (OSCULP32K)
// <GCLK_GENCTRL_SRC_OSC32K"> 32kHz High Accuracy Internal Oscillator (OSC32K)
// <GCLK_GENCTRL_SRC_XOSC32K"> 32kHz External Crystal Oscillator (XOSC32K)
// <GCLK_GENCTRL_SRC_OSC16M"> 16MHz Internal Oscillator (OSC16M)
// <GCLK_GENCTRL_SRC_DFLL48M"> Digital Frequency Locked Loop (DFLL48M)
// <GCLK_GENCTRL_SRC_DPLL96M"> Digital Phase Locked Loop (DPLL96M)
// <i> This defines the clock source for generic clock generator 7
// <id> gclk_gen_7_oscillator
#ifndef CONF_GCLK_GEN_7_SOURCE
#define CONF_GCLK_GEN_7_SOURCE GCLK_GENCTRL_SRC_OSC16M
#endif

// <q> Run in Standby
// <i> Indicates whether Run in Standby is enabled or not
// <id> gclk_arch_gen_7_runstdby
#ifndef CONF_GCLK_GEN_7_RUNSTDBY
#define CONF_GCLK_GEN_7_RUNSTDBY 0
#endif

// <q> Divide Selection
// <i> Indicates whether Divide Selection is enabled or not
//<id> gclk_gen_7_div_sel
#ifndef CONF_GCLK_GEN_7_DIVSEL
#define CONF_GCLK_GEN_7_DIVSEL 0
#endif

// <q> Output Enable
// <i> Indicates whether Output Enable is enabled or not
// <id> gclk_arch_gen_7_oe
#ifndef CONF_GCLK_GEN_7_OE
#define CONF_GCLK_GEN_7_OE 0
#endif

// <q> Output Off Value
// <i> Indicates whether Output Off Value is enabled or not
// <id> gclk_arch_gen_7_oov
#ifndef CONF_GCLK_GEN_7_OOV
#define CONF_GCLK_GEN_7_OOV 0
#endif

// <q> Improve Duty Cycle
// <i> Indicates whether Improve Duty Cycle is enabled or not
// <id> gclk_arch_gen_7_idc
#ifndef CONF_GCLK_GEN_7_IDC
#define CONF_GCLK_GEN_7_IDC 0
#endif

// <q> Generic Clock Generator Enable
// <i> Indicates whether Generic Clock Generator Enable is enabled or not
// <id> gclk_arch_gen_7_enable
#ifndef CONF_GCLK_GEN_7_GENEN
#define CONF_GCLK_GEN_7_GENEN 1
#endif
// </h>

//<h> Generic Clock Generator Division
//<o> Generic clock generator 7 division <0x0000-0xFFFF>
// <id> gclk_gen_7_div
#ifndef CONF_GCLK_GEN_7_DIV
#define CONF_GCLK_GEN_7_DIV 1
#endif
// </h>
// </e>

// <<< end of configuration section >>>

#endif // HPL_GCLK_CONFIG_H

#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

#define DEAD_BEEF	0xDEADBEEF	/**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#define RTC_FREQUENCY 100 /* Hz */

#endif /* _APP_CONFIG_H */

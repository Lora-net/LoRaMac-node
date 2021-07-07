/**
 * \file
 *
 * \brief Arch file for SAM0.
 *
 * This file defines common SAM0 series.
 *
 * Copyright (c) 2012-2020 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _SAM_IO_
#define _SAM_IO_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* SAM D20 family */
#if (SAMD20)
#  include "samd20.h"
#endif

#if (SAMD21)
#  include "samd21.h"
#endif

#if (SAMR21)
#  include "samr21.h"
#endif

#if (SAMD09)
#  include "samd09.h"
#endif

#if (SAMD10)
#  include "samd10.h"
#endif

#if (SAMD11)
#  include "samd11.h"
#endif

#if (SAML21)
#  include "saml21.h"
#endif

#if (SAMR30)
#  include "samr30.h"
#endif

#if (SAMR34)
#  include "samr34.h"
#endif

#if (WLR089)
#  include "wlr089.h"
#endif

#if (SAMR35)
#  include "samr35.h"
#endif

#if (SAML22)
#  include "saml22.h"
#endif

#if (SAMDA1)
#  include "samda1.h"
#endif

#if (SAMC20)
#  include "samc20.h"
#endif

#if (SAMC21)
#  include "samc21.h"
#endif

#if (SAMHA1)
#  include "samha1.h"
#endif

#if (SAMHA0)
#  include "samha0.h"
#endif

#if (SAMB11)
#  include "samb11.h"
#endif

#endif /* _SAM_IO_ */

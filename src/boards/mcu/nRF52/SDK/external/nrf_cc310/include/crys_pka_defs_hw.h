// Copyright (c) 2016-2017, ARM Limited or its affiliates. All rights reserved 
// 
// This file and the related binary are licensed under the ARM Object Code and 
// Headers License; you may not use these files except in compliance with this 
// license. 
// 
// You may obtain a copy of the License at <.../external/nrf_cc310/license.txt> 
// 
// See the License for the specific language governing permissions and 
// limitations under the License.



#ifndef _CRYS_PKA_DEFS_HW_H_
#define _CRYS_PKA_DEFS_HW_H_

#include "ssi_pal_types.h"
#include "ssi_pka_hw_plat_defs.h"

/*!
@file
@brief The file contains all of the enums and definitions that are used in the PKA related code.
*/


/* The valid key sizes in bits for RSA primitives (exponentiation) */
#define CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS ((CRYS_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS + SASI_PKA_WORD_SIZE_IN_BITS) / SASI_BITS_IN_32BIT_WORD )
#define CRYS_ECPKI_MODUL_MAX_LENGTH_IN_BITS   521

/*! size of buffers for Barrett modulus tag NP, used in PKI algorithms. */
#define CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS 5
#define CRYS_PKA_ECPKI_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS  CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS

#define CRYS_PKA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS

#define CRYS_PKA_PUB_KEY_BUFF_SIZE_IN_WORDS (2*CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS)

#define CRYS_PKA_PRIV_KEY_BUFF_SIZE_IN_WORDS (2*CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS)

#define CRYS_PKA_KGDATA_BUFF_SIZE_IN_WORDS   (3*CRYS_PKA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS)


/*! The valid maximum EC modulus size in 32-bit words. */
#define CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS  18 /*!< \internal [(CRYS_ECPKI_MODUL_MAX_LENGTH_IN_BITS + 31)/(sizeof(uint32_t)) + 1] */
#define CRYS_ECPKI_ORDER_MAX_LENGTH_IN_WORDS  (CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1)

#define CRYS_PKA_DOMAIN_BUFF_SIZE_IN_WORDS (2*CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS)

#define CRYS_PKA_EL_GAMAL_BUFF_MAX_LENGTH_IN_WORDS (4*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 4)

/* ECC NAF buffer definitions */
#define COUNT_NAF_WORDS_PER_KEY_WORD  8  /*!< \internal Change according to NAF representation (? 2)*/
#define CRYS_PKA_ECDSA_NAF_BUFF_MAX_LENGTH_IN_WORDS (COUNT_NAF_WORDS_PER_KEY_WORD*CRYS_ECPKI_ORDER_MAX_LENGTH_IN_WORDS + 1)

#ifndef SSI_SUPPORT_ECC_SCA_SW_PROTECT
/* on fast SCA non protected mode required additional buffers for NAF key */
#define CRYS_PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS (CRYS_PKA_ECDSA_NAF_BUFF_MAX_LENGTH_IN_WORDS+CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS+2)
#else
#define CRYS_PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS  1 /*(4*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS)*/
#endif

#define CRYS_PKA_ECPKI_BUILD_TMP_BUFF_MAX_LENGTH_IN_WORDS (3*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS+CRYS_PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)

#define CRYS_PKA_ECDSA_SIGN_BUFF_MAX_LENGTH_IN_WORDS (6*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS+CRYS_PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)

#define CRYS_PKA_ECDH_BUFF_MAX_LENGTH_IN_WORDS (2*CRYS_ECPKI_ORDER_MAX_LENGTH_IN_WORDS + CRYS_PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)

#define CRYS_PKA_KG_BUFF_MAX_LENGTH_IN_WORDS (2*CRYS_ECPKI_ORDER_MAX_LENGTH_IN_WORDS + CRYS_PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)

#define CRYS_PKA_ECDSA_VERIFY_BUFF_MAX_LENGTH_IN_WORDS (3*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS)

/* *************************************************************************** */
/*! Definitions of maximal size of modulus buffers for CRYS_EC_MONT and EC_EDW */
#define CRYS_EC_MONT_EDW_MODULUS_MAX_SIZE_IN_BYTES  32U  /*!< \internal for Curve25519 */
#define CRYS_EC_MONT_EDW_MODULUS_MAX_SIZE_IN_WORDS   8U  /*!< \internal for Curve25519 */

#define CRYS_EC_MONT_TEMP_BUFF_SIZE_IN_32BIT_WORDS  (8 * CRYS_EC_MONT_EDW_MODULUS_MAX_SIZE_IN_WORDS)

#define CRYS_EC_EDW_TEMP_BUFF_SIZE_IN_32BIT_WORD \
     (8*CRYS_EC_MONT_EDW_MODULUS_MAX_SIZE_IN_WORDS + \
     (sizeof(CRYS_HASHUserContext_t)+SASI_32BIT_WORD_SIZE-1)/SASI_32BIT_WORD_SIZE)

#endif /*_CRYS_PKA_DEFS_HW_H_*/


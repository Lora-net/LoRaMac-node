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

/*!
 * \file ssi_bitops.h
 * Bit fields operations macros.
 */
#ifndef _SSI_BITOPS_H_
#define _SSI_BITOPS_H_

#define BITMASK(mask_size) (((mask_size) < 32) ?	\
	((1UL << (mask_size)) - 1) : 0xFFFFFFFFUL)
#define BITMASK_AT(mask_size, mask_offset) (BITMASK(mask_size) << (mask_offset))

#define BITFIELD_GET(word, bit_offset, bit_size) \
	(((word) >> (bit_offset)) & BITMASK(bit_size))
#define BITFIELD_SET(word, bit_offset, bit_size, new_val)   do {    \
	word = ((word) & ~BITMASK_AT(bit_size, bit_offset)) |	    \
		(((new_val) & BITMASK(bit_size)) << (bit_offset));  \
} while (0)

/* Is val aligned to "align" ("align" must be power of 2) */
#ifndef IS_ALIGNED
#define IS_ALIGNED(val, align)		\
	(((uint32_t)(val) & ((align) - 1)) == 0)
#endif

#define SWAP_ENDIAN(word)		\
	(((word) >> 24) | (((word) & 0x00FF0000) >> 8) | \
	(((word) & 0x0000FF00) << 8) | (((word) & 0x000000FF) << 24))

#ifdef BIG__ENDIAN
#define SWAP_TO_LE(word) SWAP_ENDIAN(word)
#define SWAP_TO_BE(word) word
#else
#define SWAP_TO_LE(word) word
#define SWAP_TO_BE(word) SWAP_ENDIAN(word)
#endif



/* Is val a multiple of "mult" ("mult" must be power of 2) */
#define IS_MULT(val, mult)              \
	(((val) & ((mult) - 1)) == 0)

#define IS_NULL_ADDR(adr)		\
	(!(adr))

#endif /*_SSI_BITOPS_H_*/

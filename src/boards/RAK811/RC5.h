#ifndef __RC5_H__
#define __RC5_H__

#include <stdint.h>
#include <stdlib.h>

#define RC5_W			32	/* width in bits */

#if   (RC5_W == 16)
	#define RC5_WORD_t	uint16_t
	#define RC5_P		((RC5_WORD_t) 0xB7E1)
	#define RC5_Q		((RC5_WORD_t) 0x9E37)

#elif (RC5_W == 32)
	#define RC5_WORD_t	uint32_t
	#define RC5_P		((RC5_WORD_t) 0xB7E15163)
	#define RC5_Q		((RC5_WORD_t) 0x9E3779B9)

#elif (RC5_W == 64)
	#define RC5_WORD_t	uint64_t
	#define RC5_P		((RC5_WORD_t) 0xB7E151628AED2A6B)
	#define RC5_Q		((RC5_WORD_t) 0x9E3779B97F4A7C15)
#else
	#error "RC5_W must be defined 16, 32 or 64"

#endif

#define RC5_W_MASK		(RC5_W - 1)		/* mask */
#define RC5_W_BYTES		(RC5_W / 8)		/* width in bytes */
#define RC5_R			5				/* rounds (iterations) */
#define RC5_B			16				/* key length in bytes, round to 4 */
#define RC5_c			(RC5_B / RC5_W_BYTES)
#define RC5_S			(2 * RC5_R + 1)

//	Calculate number of blocks from bytes
#define RC5_BLOCKS(bytes)	((bytes + (2 * RC5_W_BYTES - 1)) / (2 * RC5_W_BYTES))

typedef struct {
	RC5_WORD_t W0;
	RC5_WORD_t W1;
} RC5_BLOCK_t;

void RC5_Init(const uint8_t * key);
int RC5_Blocks(int bytes);
int RC5_Encode(RC5_BLOCK_t * dst, uint8_t * src, int bytes);
void RC5_Decode(RC5_BLOCK_t * src, int blocks);
RC5_WORD_t RC5_lshift(RC5_WORD_t data, int n);
RC5_WORD_t RC5_rshift(RC5_WORD_t data, int n);

#endif /* __RC5_H__ */

/**
	Implement RC5 crypto algorithm

	https://en.wikipedia.org/wiki/RC5
 */

#include "RC5.h"

#if RC5_R > 0
	static RC5_WORD_t RC5_L_BUFFER[RC5_c];
	static RC5_WORD_t RC5_S_BUFFER[RC5_S];
#endif

/**
  * @brief	Calculate number of blocks 
  * @param	bytes	number of bytes
  * @retval	number of blocks
  */
int RC5_Blocks(int bytes)
{
	return ((bytes + (2 * RC5_W_BYTES - 1)) / (2 * RC5_W_BYTES));
}

RC5_WORD_t RC5_lshift(RC5_WORD_t data, int n)
{
	n &= RC5_W_MASK;
	if (n != 0)
		data = (data << n) | (data >> (RC5_W - n));
	return data;
}

RC5_WORD_t RC5_rshift(RC5_WORD_t data, int n)
{
	n &= RC5_W_MASK;
	if (n != 0)
		data = (data >> n) | (data << (RC5_W - n));
	return data;
}

/**
  * @brief	Initialize RC5
  * @param	key		secret key (up to RC5_B bytes)
  * @retval	none
  */
void RC5_Init(const uint8_t * key)
{
#if RC5_R > 0

	uint8_t * key_u8 = (uint8_t *)key;
	int i_s, j_l, m, k;
	RC5_WORD_t A, B;

	// Fill L from key with zeros
	k = 0;
	while (*key_u8 != '\0')
	{
		if (k < RC5_B)
			RC5_L_BUFFER[k++] = *key_u8++;
		else
			break;
	}
	while (k < RC5_B)
		RC5_L_BUFFER[k++] = 0;

	// Initialize S
	A = RC5_P;
	for (k = 0; k < RC5_S; k++)
	{
		RC5_S_BUFFER[k] = A;
		A += RC5_Q;
	}

	// Shuffle
	m = 3 * ((RC5_c > RC5_S) ? RC5_c : RC5_S);
	i_s = j_l = A = B = 0;
	for (k = 0; k < m; k++)
	{
		A = RC5_S_BUFFER[i_s] = RC5_lshift((RC5_S_BUFFER[i_s] + A + B), 3 );
		B = RC5_L_BUFFER[j_l] = RC5_lshift((RC5_L_BUFFER[j_l] + A + B), A + B);
		i_s = (i_s + 1) % RC5_S;
		j_l = (j_l + 1) % RC5_c;
	}
#endif
}

static RC5_WORD_t RC5_Read(uint8_t * src, int bytes)
{
	RC5_WORD_t data;
	int k;

	if (bytes >= sizeof(RC5_WORD_t))
		return *((RC5_WORD_t *)src);

	data = 0;
	k = 0;
	while (bytes != 0)
	{
		--bytes;
		data |= ((RC5_WORD_t)(*src++)) << k;
		k += 8;
	}
	return data;
}

/**
  * @brief	Encode data
  * @param	dst		pointer to destination blocks
  * @param	src		pointer to source bytes
  * @param	bytes	source length in bytes
  * @retval	number of blocks
  */
int RC5_Encode(RC5_BLOCK_t * dst, uint8_t * src, int bytes)
{
	RC5_WORD_t A, B;
	int blocks = 0;

#if RC5_R > 0
	RC5_WORD_t *s;
#endif

	while (bytes != 0)
	{
		A = B = 0;
		if (bytes >= sizeof(RC5_WORD_t))
		{
			A = *((RC5_WORD_t *)src);
			src += sizeof(RC5_WORD_t);
			bytes -= sizeof(RC5_WORD_t);
		}
		else
		{
			A = RC5_Read(src, bytes);
			bytes = 0;
		}

		if (bytes >= sizeof(RC5_WORD_t))
		{
			B = *((RC5_WORD_t *)src);
			src += sizeof(RC5_WORD_t);
			bytes -= sizeof(RC5_WORD_t);
		}
		else
		{
			B = RC5_Read(src, bytes);
			bytes = 0;
		}

#if RC5_R > 0
		s = &RC5_S_BUFFER[0];
		A = A + *(s++);
		B = B + *(s++);

#if RC5_R > 1
		for (int i = 1; i < RC5_R; i++)
		{
			A = RC5_lshift((A ^ B), B) + *(s++);
			B = RC5_lshift((A ^ B), A) + *(s++);
		}
#endif
#endif
		dst->W0 = A;
		dst->W1 = B;
		dst++;
		blocks++;
	}
	return blocks;
}

/**
  * @brief	Encode data
  * @param	dst		pointer to destination blocks
  * @param	src		pointer to source bytes
  * @param	bytes	source length in bytes
  * @retval	number of blocks
  */
void RC5_Decode(RC5_BLOCK_t * src, int blocks)
{
	RC5_WORD_t A, B;

#if RC5_R > 0
	RC5_WORD_t *s;
#endif
	while (blocks != 0)
	{
		--blocks;
		A = src->W0;
		B = src->W1;

#if RC5_R > 0
		s = &RC5_S_BUFFER[RC5_S - 1];

#if RC5_R > 1
		for (int i = RC5_R - 1; i != 0; i--)
		{
			B = RC5_rshift(B - *(--s), A) ^ A;
			A = RC5_rshift(A - *(--s), B) ^ B;
		}
#endif
		B = B - *(--s);
		A = A - *(--s);
#endif
		src->W0 = A;
		src->W1 = B;
		src++;
	}
}

/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Helper functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

/*!
 * \brief Returns the minimum value between a and b
 *
 * \param [IN] a 1st value
 * \param [IN] b 2nd value
 * \retval minValue Minimum value
 */
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

/*!
 * \brief Returns the maximum value between a and b
 *
 * \param [IN] a 1st value
 * \param [IN] b 2nd value
 * \retval maxValue Maximum value
 */
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

/*!
 * \brief Returns 2 raised to the power of n
 *
 * \param [IN] n power value
 * \retval result of raising 2 to the power n
 */
#define POW2( n ) ( 1 << n )

/*!
 * \brief Returns num / den with any fractional part discarded
 *
 * \param [IN] num the numerator, which can have any sign
 * \param [IN] den the denominator, which can have any sign
 * \retval num / den with any fractional part discarded
 *
 * According to C standard:
 *  when den != 0, this is the definition of num / den
 *  when den == 0, the result is undefined
 *
 * Examples:  num den | result
 *            +4  +4  | +1
 *            +4  -4  | -1
 *            -4  +4  | -1
 *            -4  -4  | +1
 *            +5  +4  | +1
 *            +5  -4  | -1
 *            -5  +4  | -1
 *            -5  -4  | +1
 *            +6  +4  | +1
 *            +6  -4  | -1
 *            -6  +4  | -1
 *            -6  -4  | +1
 *            +7  +4  | +1
 *            +7  -4  | -1
 *            -7  +4  | -1
 *            -7  -4  | +1
 *            +8  +4  | +2
 *            +8  -4  | -2
 *            -8  +4  | -2
 *            -8  -4  | +2
 *
 * The _BOTH_POSTITIVE_SIGNS version exists to suppress compiler warnings if it
 * is known that both arguments are postive
 */
#define INT_CLOSEST_TO_ZERO_DIV_ANY_SIGNS( num, den )           \
  ( ( num ) / ( den ) )

#define INT_CLOSEST_TO_ZERO_DIV_BOTH_POSTITIVE_SIGNS( num, den )  \
  ( ( num ) / ( den ) )

/*!
 * \brief Returns num / den with any fractional part rounded to nearest infinity
 *
 * \param [IN] num the numerator, which can have any sign
 * \param [IN] den the denominator, which can have any sign
 * \retval num / den with any fractional part rounded to nearest infinity
 *
 *  when den == 0, the result is undefined
 *
 * Note, according to C standard, when num / den is representable,
 * ( num / den ) * den + num % den == num.
 * This implies num % den == num - ( num / den ) * den.
 * Therefore:  num den | num % den
 *            +7  +4  | +3
 *            +7  -4  | +3
 *            -7  +4  | -3
 *            -7  -4  | -3
 *
 * The results of this macro then produce
 * Examples:  num den | result
 *            +4  +4  | +1
 *            +4  -4  | -1
 *            -4  +4  | -1
 *            -4  -4  | +1
 *            +5  +4  | +2
 *            +5  -4  | -2
 *            -5  +4  | -2
 *            -5  -4  | +2
 *            +6  +4  | +2
 *            +6  -4  | -2
 *            -6  +4  | -2
 *            -6  -4  | +2
 *            +7  +4  | +2
 *            +7  -4  | -2
 *            -7  +4  | -2
 *            -7  -4  | +2
 *            +8  +4  | +2
 *            +8  -4  | -2
 *            -8  +4  | -2
 *            -8  -4  | +2
 *
 * Note the optimizer on the compiler should compute a/b and a%b simultaneously,
 * and only use these subexpressions further on, so this is as tight as possible
 *
 * The _BOTH_POSTITIVE_SIGNS version exists to suppress compiler warnings if it
 * is known that both arguments are postive
 */
/*
 * macros to enable or diable warings about pointless comparison of unsigned
 * quantities with zero in a compiler-specific way
 */
#define INT_CLOSEST_TO_INFINITIES_DIV_ANY_SIGNS( num, den )                 \
  (                                                                         \
    (                                                                       \
      ( ( ( ( num ) / ( den ) ) >= 0 ) && ( ( ( num ) % ( den ) ) >= 0 ) )  \
      ||                                                                    \
      ( ( ( ( num ) / ( den ) ) <= 0 ) && ( ( ( num ) % ( den ) ) <= 0 ) )  \
    )                                                                       \
    ? ( ( ( num ) / ( den ) ) + ( ( ( num ) % ( den ) ) ? 1 : 0) )          \
    : ( ( ( num ) / ( den ) ) - ( ( ( num ) % ( den ) ) ? 1 : 0) )          \
  )

#define INT_CLOSEST_TO_INFINITIES_DIV_BOTH_POSTITIVE_SIGNS( num, den )      \
  ( ( ( num ) / ( den ) ) + ( ( ( num ) % ( den ) ) ? 1 : 0) )

/*!
 * \brief Returns the closest integer to num / den
 *
 * \param [IN] num the numerator, which can have any sign
 * \param [IN] den the denominator, which can have any sign
 * \retval closest integer to num / den
 *
 *  when den == 0, the result is undefined
 *
 * Note, according to C standard, when num / den is representable,
 * ( num / den ) * den + num % den == num.
 * This implies num % den == num - ( num / den ) * den.
 * Therefore:  num den | num % den
 *            +7  +4  | +3
 *            +7  -4  | +3
 *            -7  +4  | -3
 *            -7  -4  | -3
 *
 * The results of this macro then produce
 * Examples:  num den | result
 *            +4  +4  | +1
 *            +4  -4  | -1
 *            -4  +4  | -1
 *            -4  -4  | +1
 *            +5  +4  | +1
 *            +5  -4  | -1
 *            -5  +4  | -1
 *            -5  -4  | +1
 *            +6  +4  | +2
 *            +6  -4  | -2
 *            -6  +4  | -2
 *            -6  -4  | +2
 *            +7  +4  | +2
 *            +7  -4  | -2
 *            -7  +4  | -2
 *            -7  -4  | +2
 *            +8  +4  | +2
 *            +8  -4  | -2
 *            -8  +4  | -2
 *            -8  -4  | +2
 *
 * Note the optimizer on the compiler should compute a/b and a%b simultaneously,
 * and only use these subexpressions further on, so this is as tight as possible
 *
 * The _BOTH_POSTITIVE_SIGNS version exists to suppress compiler warnings if it
 * is known that both arguments are postive
 */
#define INT_CLOSEST_TO_DIV_ANY_SIGNS( num, den )                            \
  (                                                                         \
    (                                                                       \
      ( ( ( num ) >= 0 ) && ( ( ( ( num ) % ( den ) ) * 2 ) >= ( den ) ) )  \
      ||                                                                    \
      ( ( ( num ) <= 0 ) && ( ( ( ( num ) % ( den ) ) * 2 ) <= ( den ) ) )  \
    )                                                                       \
    ? ( INT_CLOSEST_TO_INFINITIES_DIV_ANY_SIGNS( ( num ), ( den ) ) )       \
    : ( INT_CLOSEST_TO_ZERO_DIV_ANY_SIGNS( ( num ), ( den ) ) )             \
  )

#define INT_CLOSEST_TO_DIV_BOTH_POSTITIVE_SIGNS( num, den )                       \
  (                                                                               \
    ( ( ( ( num ) % ( den ) ) * 2 ) >= ( den ) )                                  \
    ? ( INT_CLOSEST_TO_INFINITIES_DIV_BOTH_POSTITIVE_SIGNS( ( num ), ( den ) ) )  \
    : ( INT_CLOSEST_TO_ZERO_DIV_BOTH_POSTITIVE_SIGNS( ( num ), ( den ) ) )        \
  )

/*!
 * \brief Initializes the pseudo random generator initial value
 *
 * \param [IN] seed Pseudo random generator initial value
 */
void srand1( uint32_t seed );

/*!
 * \brief Computes a random number between min and max
 *
 * \param [IN] min range minimum value
 * \param [IN] max range maximum value
 * \retval random random value in range min..max
 */
int32_t randr( int32_t min, int32_t max );

/*!
 * \brief Copies size elements of src array to dst array
 *
 * \remark STM32 Standard memcpy function only works on pointers that are aligned
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size );

/*!
 * \brief Copies size elements of src array to dst array reversing the byte order
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size );

/*!
 * \brief Set size elements of dst array with value
 *
 * \remark STM32 Standard memset function only works on pointers that are aligned
 *
 * \param [OUT] dst   Destination array
 * \param [IN]  value Default value
 * \param [IN]  size  Number of bytes to be copied
 */
void memset1( uint8_t *dst, uint8_t value, uint16_t size );

/*!
 * \brief Converts a nibble to an hexadecimal character
 *
 * \param [IN] a   Nibble to be converted
 * \retval hexChar Converted hexadecimal character
 */
int8_t Nibble2HexChar( uint8_t a );

#endif // __UTILITIES_H__

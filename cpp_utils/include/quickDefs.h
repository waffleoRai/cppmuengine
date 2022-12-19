/***************************************************************************//**
@file quickDefs.h
@brief Some useful typedefs and macros.

This header contains macro definitions and typedefs that are useful for shorthand
and making code clearer. 

Includes:
+ stdint

@author Blythe Hospelhorn
@version 1.0.0
@since December 4, 2022

 ******************************************************************************/

#ifndef QUICKDEFS_H_INCLUDED
#define QUICKDEFS_H_INCLUDED

#include <stdint.h>

#define TRUE 1 /**< Pseudo-boolean "true" value for use in C. */
#define FALSE 0 /**< Pseudo-boolean "false" value for use in C. */

typedef unsigned char uchar; /**< Shorthand typedef for `unsigned char` */
typedef unsigned short ushort; /**< Shorthand typedef for `unsigned short` */
typedef unsigned int uint; /**< Shorthand typedef for `unsigned int` */
typedef unsigned long ulong; /**< Shorthand typedef for `unsigned long` */
typedef unsigned long long ullong; /**< Shorthand typedef for `unsigned long long` */

typedef int8_t s8; /**< Shorthand typedef for signed 8-bit fixed width value. */
typedef int16_t s16; /**< Shorthand typedef for signed 16-bit fixed width value. */
typedef int32_t s32; /**< Shorthand typedef for signed 32-bit fixed width value. */
typedef int64_t s64; /**< Shorthand typedef for signed 64-bit fixed width value. */

typedef uint8_t u8; /**< Shorthand typedef for unsigned 8-bit fixed width value (eg. byte). */
typedef uint16_t u16; /**< Shorthand typedef for unsigned 16-bit fixed width value. */
typedef uint32_t u32; /**< Shorthand typedef for unsigned 32-bit fixed width value. */
typedef uint64_t u64; /**< Shorthand typedef for unsigned 64-bit fixed width value. */

typedef uint8_t ubyte; /**< Unsigned byte. (Note: "byte" was not used due to conflicts with other libraries' use of it.)*/
typedef int boolean; /**< True/false value stored and passed as a signed int.*/

#endif // QUICKDEFS_H_INCLUDED

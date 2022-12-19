/***************************************************************************//**
@file wr_c_utils.h
@brief C-compatible general use macros and functions.

WaffleoRaiCUtils (WRCU):
A set of macros and functions for common use. Includes some handling for Unicode
strings, based upon the [ICU] C library.

Includes:
+ stdint
+ string
+ stdlib
+ [ICU] uchar
+ [ICU] unicode/ucnv.h

[ICU]: https://unicode-org.github.io/icu/ "ICU"

@author Blythe Hospelhorn
@version 1.0.0
@since December 3, 2022

 ******************************************************************************/

#ifndef WR_C_UTILS_H_INCLUDED
#define WR_C_UTILS_H_INCLUDED

#include "quickDefs.h"
#include <string.h>
#include <uchar.h>
#include <stdlib.h>
#include <time.h>

#include "unicode/ucnv.h"

#define SIZE_UNKNOWN ~0ULL /**< Long-long value for indicating an unknown size or position (for types such as size_t or streampos). */

#define WRCU_ICU_ENAME_UTF8 "UTF8" /**< ICU encoding string for UTF-8 */
#define WRCU_ICU_ENAME_UTF16BE "UTF16BE" /**< ICU encoding string for Big-Endian UTF-16 */
#define WRCU_ICU_ENAME_UTF16LE "UTF16LE" /**< ICU encoding string for Little-Endian UTF-16 */
#define WRCU_ICU_ENAME_UTF32BE "UTF32BE" /**< ICU encoding string for Big-Endian UTF-32 */
#define WRCU_ICU_ENAME_UTF32LE "UTF32LE" /**< ICU encoding string for Little-Endian UTF-32 */

#define SYS_BIG_ENDIAN !*((uint8_t*)&((uint16_t){1})) /**< A macro for run-time byte order checking, checks whether system memory uses Big-Endian byte ordering. Recommended use _TARGET_BE macro for compile-time checking instead. */

//Also use macros _TARGET_LE and _TARGET_BE

#ifdef _WIN32
#    ifdef WRCU_BUILD
#        define WRCU_DLL_API __declspec(dllexport) /**< WaffleoRai C Utils DLL export macro for Windows builds. */
#    else
#        define WRCU_DLL_API __declspec(dllimport) /**< WaffleoRai C Utils DLL import macro for Windows builds. */
#    endif
#	define WRCU_CDECL __cdecl /**< Macro to specify C-style calling convention. */
#	define WRCU_STDCALL __stdcall /**< Macro to specify standard calling convention. */
#	define FILE_SEP '\\' /**< System file separator character. Backslash on Windows. */
#else
#   define WRCU_DLL_API /**< WaffleoRai C Utils DLL macro for Windows builds. Left empty for non-Windows builds.*/
#	define WRCU_CDECL  /**< Macro to specify C-style calling convention. */
#	define WRCU_STDCALL /**< Macro to specify standard calling convention. */
#	define FILE_SEP '/' /**< System file separator character.*/
#endif

#ifdef _TARGET_BE
#	define READ_16_BE(dst, src) *((uint16_t*)dst) = *((uint16_t*)src)
#	define READ_32_BE(dst, src) *((uint32_t*)dst) = *((uint32_t*)src)
#	define READ_64_BE(dst, src) *((uint64_t*)dst) = *((uint64_t*)src)

#	define READ_16_LE(dst, src) *dst = *(src+1); \
								*(dst + 1) = *src;

#	define READ_32_LE(dst, src) *dst = *(src+3); \
							    *(dst+1) = *(src+2); \
							    *(dst+2) = *(src+1); \
							    *(dst+3) = *src;

#	define READ_64_LE(dst, src) *dst = *(src+7); \
							    *(dst+1) = *(src+6); \
							    *(dst+2) = *(src+5); \
							    *(dst+3) = *(src+4); \
							    *(dst+4) = *(src+3); \
							    *(dst+5) = *(src+2); \
							    *(dst+6) = *(src+1); \
							    *(dst+7) = *src;
#else
#	define READ_16_LE(dst, src) *((uint16_t*)dst) = *((uint16_t*)src)
#	define READ_32_LE(dst, src) *((uint32_t*)dst) = *((uint32_t*)src)
#	define READ_64_LE(dst, src) *((uint64_t*)dst) = *((uint64_t*)src)

#	define READ_16_BE(dst, src) *dst = *(src+1); \
								*(dst + 1) = *src;

#	define READ_32_BE(dst, src) *dst = *(src+3); \
							    *(dst+1) = *(src+2); \
							    *(dst+2) = *(src+1); \
							    *(dst+3) = *src;

#	define READ_64_BE(dst, src) *dst = *(src+7); \
							    *(dst+1) = *(src+6); \
							    *(dst+2) = *(src+5); \
							    *(dst+3) = *(src+4); \
							    *(dst+4) = *(src+3); \
							    *(dst+5) = *(src+2); \
							    *(dst+6) = *(src+1); \
							    *(dst+7) = *src;

#endif

//For utils defined in C

#ifdef __cplusplus
extern "C" {
#endif

	extern const char16_t FILE_SEP16; /**< System's file path separator as a wide character.*/

	void WRCU_CDECL calculateTimezoneOffset(); /**< Recalculate the time difference between the system's time zone and GMT.*/

	/**
	Get the time difference between the system's time zone and GMT.
	@return Time offset in seconds. Can be used with C time library `struct tm` instances.
	*/
	WRCU_DLL_API double WRCU_CDECL getTimezoneOffset();

	//Number/String conversion

	/**
	Convert the provided fixed-width unsigned 8-bit number to a string representing the number in base 10.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u8_to_decstr(uint8_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 16-bit number to a string representing the number in base 10.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u16_to_decstr(uint16_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 32-bit number to a string representing the number in base 10.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u32_to_decstr(uint32_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 64-bit number to a string representing the number in base 10.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u64_to_decstr(uint64_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 8-bit number to a string representing the number in base 16. The `0x` prefix is not added.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u8_to_hexstr(uint8_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 16-bit number to a string representing the number in base 16. The `0x` prefix is not added.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u16_to_hexstr(uint16_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 32-bit number to a string representing the number in base 16. The `0x` prefix is not added.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u32_to_hexstr(uint32_t value, char* dst, int mindig);

	/**
	Convert the provided fixed-width unsigned 64-bit number to a string representing the number in base 16. The `0x` prefix is not added.
	@param value The number to get a string representation of.
	@param dst The `char` buffer to write the string to.
	@param mindig The minimum number of digits that should be used in the string. Zero digits will be added to the left until the minimum is reached.
	@return The length of the generated string.
	*/
	WRCU_DLL_API const int WRCU_CDECL u64_to_hexstr(uint64_t value, char* dst, int mindig);

	//Unicode

	/**
	Create and open a `UConverter` preconfigured to encode and decode UTF-8. 
	
	* **Note:** This is a convenience function that wraps `ucnv_open`, so unless `ucnv_close` is called on the returned `UConverter` pointer to
	free it, a memory leak may result.
	@param err A pointer to memory where an error code may be written to if an error occurs during opening.
	@return A pointer to the opened `UConverter` structure.
	*/
	WRCU_DLL_API UConverter* WRCU_CDECL new_utf8_conv(UErrorCode* err);

	/**
	Create and open a `UConverter` preconfigured to encode and decode UTF-32. The byte-order will match the system's.

	* **Note:** This is a convenience function that wraps `ucnv_open`, so unless `ucnv_close` is called on the returned `UConverter` pointer to
	free it, a memory leak may result.
	@param err A pointer to memory where an error code may be written to if an error occurs during opening.
	@return A pointer to the opened `UConverter` structure.
	*/
	WRCU_DLL_API UConverter* WRCU_CDECL new_utf32_conv(UErrorCode* err);

	/**
	Convert an encoded input string to a string of UTF-16 wide characters in the system's byte-order, ready for use by other ICU modules or any library
	that utilizes UTF-16 as its in-memory string encoding.
	@param dst The destination buffer to write the UTF-16 string to.
	@param src The source buffer containing the encoded characters to convert.
	@param sz_dst The capacity, in number of wide-characters, of the destination buffer.
	@param sz_src The length of the input string, in character units of input encoding.
	@param conv The pre-opened `UConverter` to use to read the encoded input.
	@param err A pointer to memory where an error code may be written to if an error occurs during conversion.
	@return The length of the output string, in wide-characters. This does not count the terminating `\0` character.
	*/
	WRCU_DLL_API const int32_t WRCU_CDECL to_utf16(char16_t* dst, const char* src, int32_t sz_dst, int32_t sz_src, UConverter* conv, UErrorCode* err);

	/**
	Convert a UTF-8 encoded string to a UTF-32 string of 32-bit wide characters (ie. the system's byte-order is used). 
	@param dst The destination buffer to write the UTF-32 string to.
	@param src The source buffer containing the UTF-8 characters to convert.
	@param sz_dst The capacity, in bytes, of the destination buffer.
	@param sz_src The length of the input string, bytes.
	@param err A pointer to memory where an error code may be written to if an error occurs during conversion.
	@return The length of the output string, in wide-characters. This does not count the terminating `\0` character.
	*/
	WRCU_DLL_API const int32_t WRCU_CDECL utf8_to_utf32(char32_t* dst, const char* src, int32_t sz_dst, int32_t sz_src, UErrorCode* err);

	//Byte Order
	/**
	Do a runtime check to determine whether the host system's byte ordering is Big-Endian.
	@return `TRUE` if the system's native byte-order is Big-Endian. `FALSE` if not (ie. the system is Little-Endian).
	*/
	WRCU_DLL_API const boolean WRCU_CDECL wrcu_sys_big_endian();

	/**
	Reverse the order of a number of bytes in memory.
	@param ptr Pointer to the start of bytes to reverse.
	@param nbytes Number of bytes from provided address to reverse.
	*/
	WRCU_DLL_API void WRCU_CDECL wrcu_reverseBytes(uint8_t* ptr, const int nbytes);

#ifdef __cplusplus
}
#endif

#endif // WR_C_UTILS_H_INCLUDED

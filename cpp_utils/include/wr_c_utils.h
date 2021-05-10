#ifndef WR_C_UTILS_H_INCLUDED
#define WR_C_UTILS_H_INCLUDED

#include "quickDefs.h"
#include <string.h>
#include <uchar.h>
#include <stdlib.h>

#include "unicode/ucnv.h"

#define WRCU_ICU_ENAME_UTF8 "UTF8"
#define WRCU_ICU_ENAME_UTF32BE "UTF32BE"
#define WRCU_ICU_ENAME_UTF32LE "UTF32LE"

#define SYS_BIG_ENDIAN !*((uint8_t*)&((uint16_t){1}))

#ifdef _WIN32
#    ifdef WRCU_BUILD
#        define WRCU_DLL_API __declspec(dllexport)
#    else
#        define WRCU_DLL_API __declspec(dllimport)
#    endif
#	define WRCU_CDECL __cdecl
#	define WRCU_STDCALL __stdcall
#	define FILE_SEP '\\'
#elif
#   define WRCU_DLL_API
#	define WRCU_CDECL
#	define WRCU_STDCALL
#	define FILE_SEP '/'
#endif

//For utils defined in C

#ifdef __cplusplus
extern "C" {
#endif

	extern const char16_t FILE_SEP16;

	//Number/String conversion

	WRCU_DLL_API const int WRCU_CDECL u8_to_decstr(uint8_t value, char* dst, int mindig);
	WRCU_DLL_API const int WRCU_CDECL u16_to_decstr(uint16_t value, char* dst, int mindig);
	WRCU_DLL_API const int WRCU_CDECL u32_to_decstr(uint32_t value, char* dst, int mindig);
	WRCU_DLL_API const int WRCU_CDECL u64_to_decstr(uint64_t value, char* dst, int mindig);

	WRCU_DLL_API const int WRCU_CDECL u8_to_hexstr(uint8_t value, char* dst, int mindig);
	WRCU_DLL_API const int WRCU_CDECL u16_to_hexstr(uint16_t value, char* dst, int mindig);
	WRCU_DLL_API const int WRCU_CDECL u32_to_hexstr(uint32_t value, char* dst, int mindig);
	WRCU_DLL_API const int WRCU_CDECL u64_to_hexstr(uint64_t value, char* dst, int mindig);

	//Unicode

	WRCU_DLL_API UConverter* WRCU_CDECL new_utf8_conv(UErrorCode* err);
	WRCU_DLL_API UConverter* WRCU_CDECL new_utf32_conv(UErrorCode* err);
	WRCU_DLL_API const int32_t WRCU_CDECL to_utf16(char16_t* dst, const char* src, int32_t sz_dst, int32_t sz_src, const UConverter* conv, UErrorCode* err);
	WRCU_DLL_API const int32_t WRCU_CDECL utf8_to_utf32(char32_t* dst, const char* src, int32_t sz_dst, int32_t sz_src, UErrorCode* err);

	//Byte Order
	WRCU_DLL_API const boolean WRCU_CDECL wrcu_sys_big_endian();
	WRCU_DLL_API void WRCU_CDECL wrcu_reverseBytes(uint8_t* ptr, const int nbytes);

#ifdef __cplusplus
}
#endif

#endif // WR_C_UTILS_H_INCLUDED

#ifndef AES_C_H_INCLUDED
#define AES_C_H_INCLUDED

//Put the actual AES stuff here - in C

#include "quickDefs.h"
#include "muenDefs.h"
#include <stdint.h>
#include <string.h>

#define AES_KEYBYTES_128 16
#define AES_KEYSLOTS_128 11
#define AES_ROUNDS_128 9

#ifdef __cplusplus
extern "C" {
#endif

//https://www.samiam.org/galois.html
    WRMUENAM_DLL_API extern const int32_t AES_TBL_LOG[256];
    WRMUENAM_DLL_API extern const int32_t AES_TBL_ANTILOG[256];
    WRMUENAM_DLL_API extern const int32_t AES_SHIFT_ROWS_MAP[16];

    WRMUENAM_DLL_API extern boolean AES_TABLESINIT;
    WRMUENAM_DLL_API int32_t AES_RCON[256];
    WRMUENAM_DLL_API int32_t AES_SBOX[256];
    WRMUENAM_DLL_API int32_t AES_SBOXINV[256];

typedef struct WRMUENAM_DLL_API aes_key128{

    ubyte aes_key[AES_KEYBYTES_128];
    ubyte key_sched[AES_KEYSLOTS_128* AES_KEYBYTES_128];

    boolean is_init;

} aes_key128_t;

typedef struct WRMUENAM_DLL_API aes_state128{

    aes_key128_t* key;
    ubyte* vec;

} aes_state128_t;

WRMUENAM_DLL_API const int WRMUENAM_CDECL aes_init_common_tables();
WRMUENAM_DLL_API const int32_t WRMUENAM_CDECL aes_rcon(int32_t in);
WRMUENAM_DLL_API const int32_t WRMUENAM_CDECL aes_sbox(int32_t in);

WRMUENAM_DLL_API void WRMUENAM_CDECL aes_gen_key_schedule_128(aes_key128_t* key);
WRMUENAM_DLL_API aes_key128_t* WRMUENAM_CDECL aes_gen_key_128(ubyte* rawkey);

//uint32_t aesutil_rol32(uint32_t value, int amt);
//uint32_t aesutil_ror32(uint32_t value, int amt);
WRMUENAM_DLL_API void WRMUENAM_CDECL aesutil_add128(ubyte* src1, ubyte* src2, ubyte* dst);
WRMUENAM_DLL_API void WRMUENAM_CDECL aesutil_xor128(ubyte* src1, ubyte* src2, ubyte* dst);

WRMUENAM_DLL_API const int32_t WRMUENAM_CDECL aes_gmul(int32_t a, int32_t b);
WRMUENAM_DLL_API const int WRMUENAM_CDECL rijndael_enc(aes_key128_t* key, ubyte* src, ubyte* dst);
WRMUENAM_DLL_API const int WRMUENAM_CDECL rijndael_dec(aes_key128_t* key, ubyte* src, ubyte* dst);

WRMUENAM_DLL_API const int WRMUENAM_CDECL aes_encblock_cbc128(ubyte* src, ubyte* dst, aes_state128_t* state);
WRMUENAM_DLL_API const int WRMUENAM_CDECL aes_decblock_cbc128(ubyte* src, ubyte* dst, aes_state128_t* state);

WRMUENAM_DLL_API const size_t WRMUENAM_CDECL aes_enc_cbc128(ubyte* src, ubyte* dst, const size_t length, ubyte* key, ubyte* iv);
WRMUENAM_DLL_API const size_t WRMUENAM_CDECL aes_dec_cbc128(ubyte* src, ubyte* dst, const size_t length, ubyte* key, ubyte* iv);

#ifdef __cplusplus
}
#endif

#endif // AES_C_H_INCLUDED

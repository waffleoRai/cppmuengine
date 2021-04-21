#ifndef MUENAM_FORMATS_H_INCLUDED
#define MUENAM_FORMATS_H_INCLUDED

#include "wr_c_utils.h"
#include "muenDefs.h"

#define INIBIN_HDR_FLAG_ASSHAES 0x0004
#define INIBIN_HDR_FLAG_ALLAES 0x0008

//C - structs for engine boot file formats

#ifdef __cplusplus
extern "C" {
#endif

//Everything before path table and group table
typedef WRMUENAM_DLL_API struct muen_initbin_hdr{

    uint32_t inibin_ver;
    char gamecode[8];
    uint16_t gamever_maj;
    uint16_t gamever_min;
    uint16_t gamever_bld;
    uint16_t flags;
    uint64_t last_mod;
    uint32_t rsv0;

    uint8_t aeskey[16];
    uint8_t hmackey[16];
    uint64_t  memlmt;

} muen_initbin_hdr_t;

typedef WRMUENAM_DLL_API struct muen_assh_hdr {

    char magic[4];
    uint32_t assh_ver;
    uint64_t assh_size;
    uint32_t nametbl_off;
    uint32_t assettbl_off;

} muen_assh_hdr_t;

typedef WRMUENAM_DLL_API struct muen_assh_entry {

    uint32_t type;
    uint32_t group;
    uint64_t instance;
    uint16_t flags;
    uint16_t pdg0;
    uint32_t path_idx;
    uint64_t offset;
    uint64_t pkged_size;
    uint64_t decomp_size;
    ubyte sha256[32];

} muen_assh_entry_t;

WRMUENAM_DLL_API void WRMUENAM_CDECL brev_muen_initbin_hdr(muen_initbin_hdr_t* hdr);

#ifdef __cplusplus
}
#endif

#endif // MUENAM_FORMATS_H_INCLUDED

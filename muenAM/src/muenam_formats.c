#include "muenam_formats.h"

void brev_muen_initbin_hdr(muen_initbin_hdr_t* hdr) {
	if (!hdr) return;
	wrcu_reverseBytes(&hdr->inibin_ver, 4);
	wrcu_reverseBytes(&hdr->gamever_maj, 2);
	wrcu_reverseBytes(&hdr->gamever_min, 2);
	wrcu_reverseBytes(&hdr->gamever_bld, 2);
	wrcu_reverseBytes(&hdr->flags, 2);
	wrcu_reverseBytes(&hdr->last_mod, 8);
	wrcu_reverseBytes(&hdr->memlmt, 8);
}
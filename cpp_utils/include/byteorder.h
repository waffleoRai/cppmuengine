#ifndef BYTEORDER_H_INCLUDED
#define BYTEORDER_H_INCLUDED

#include <stdint.h>

#define SYS_BIG_ENDIAN (!*((uint8_t*)&((uint16_t){1}))))

#ifdef __cplusplus
extern "C" {
#endif

	int wrcu_sys_big_endian() {
		return 0;
	}

	void wrcu_reverseBytes(uint8_t* ptr, const int nbytes) {

		if (nbytes < 2) return;

		uint8_t temp = 0;
		int i = 0;
		int j = nbytes - 1;

		while (j > i) {
			temp = *(ptr + i);
			*(ptr + i) = *(ptr + j);
			*(ptr + j) = temp;
			i++; j--;
		}

	}

#ifdef __cplusplus
}
#endif

#endif //BYTEORDER_H_INCLUDED
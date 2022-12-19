
#include "wr_c_utils.h"

//Number/String conversion

const char16_t FILE_SEP16 = (const char16_t)FILE_SEP;

int timezone_offset_set = FALSE;
double timezone_offset = 0.0;

void calculateTimezoneOffset() {
    time_t now = time(0);
    struct tm* time_ptr = NULL;
    struct tm time_gm;
    struct tm time_lc;
    time_t t_gm = 0;
    time_t t_lc = 0;

    time_ptr = gmtime(&now);
    memcpy(&time_gm, time_ptr, sizeof(struct tm));
    t_gm = mktime(&time_gm);

    time_ptr = localtime(&now);
    memcpy(&time_lc, time_ptr, sizeof(struct tm));
    t_lc = mktime(&time_lc);

    timezone_offset = difftime(t_lc, t_gm);

    timezone_offset_set = TRUE;
}

double getTimezoneOffset() {
    if (timezone_offset_set) { return timezone_offset; }
    calculateTimezoneOffset(); return timezone_offset;
}

const int u8_to_decstr(uint8_t value, char* dst, int mindig){
    return u64_to_decstr((uint64_t)value, dst, mindig);
}

const int u16_to_decstr(uint16_t value, char* dst, int mindig){
    return u64_to_decstr((uint64_t)value, dst, mindig);
}

const int u32_to_decstr(uint32_t value, char* dst, int mindig){
    return u64_to_decstr((uint64_t)value, dst, mindig);
}

const int u64_to_decstr(uint64_t value, char* dst, int mindig){
    int dct = 0;
    char* pos = dst;
    *(pos++) = '\0';

    while(value > 0){
        *(pos++) = '0' + (char)(value % 10);
        value /= 10;
    }

    dct = (int)(pos-dst-1);
    while(dct++ < mindig){
        *(pos++) = '0';
    }

    wrcu_reverseBytes((uint8_t*)dst, dct);

    return dct-1;
}

const int u8_to_hexstr(uint8_t value, char* dst, int mindig){
    return u64_to_hexstr((uint64_t)value, dst, mindig);
}

const int u16_to_hexstr(uint16_t value, char* dst, int mindig){
    return u64_to_hexstr((uint64_t)value, dst, mindig);
}

const int u32_to_hexstr(uint32_t value, char* dst, int mindig){
    return u64_to_hexstr((uint64_t)value, dst, mindig);
}

const int u64_to_hexstr(uint64_t value, char* dst, int mindig){
    char* pos = dst;
    uint64_t nyb = 0;
    *(pos++) = '\0';

    while(value > 0){
        nyb = value & 0xfULL;
        if(nyb < 10) *(pos++) = '0' + (char)nyb;
        else *(pos++) = 'a' + (char)(nyb-10);
        value >>= 4;
    }

    int dct = (int)(pos-dst-1);
    while(dct++ < mindig){
        *(pos++) = '0';
    }

    wrcu_reverseBytes((uint8_t*)dst, dct);

    return dct-1;
}


//Unicode
UConverter* new_utf8_conv(UErrorCode* err){
    return ucnv_open(WRCU_ICU_ENAME_UTF8, err);
}

UConverter* new_utf32_conv(UErrorCode* err){
    if(wrcu_sys_big_endian()) return ucnv_open(WRCU_ICU_ENAME_UTF32BE, err);
    else return ucnv_open(WRCU_ICU_ENAME_UTF32LE, err);
}


const int32_t to_utf16(char16_t* dst, const char* src, int32_t sz_dst, int32_t sz_src, UConverter* conv, UErrorCode* err){
    if(!conv || !src || !dst) return 0; //If no error, it's because null ptr
    int32_t read = ucnv_toUChars (conv, (UChar*)dst, sz_dst, src, sz_src, err);

    return read;
}

const int32_t utf8_to_utf32(char32_t* dst, const char* src, int32_t sz_dst, int32_t sz_src, UErrorCode* err){
    if(!src || !dst) return 0;
    char* toconv;
    if(wrcu_sys_big_endian()) toconv = WRCU_ICU_ENAME_UTF32BE;
    else toconv = WRCU_ICU_ENAME_UTF32LE;

    int32_t read = ucnv_convert (toconv, WRCU_ICU_ENAME_UTF8, (char*)dst, sz_dst, src, sz_src, err);

    return read;
}


//Byte Order
int wrcu_be_detected = -1;

const boolean wrcu_sys_big_endian() {
    if (wrcu_be_detected < 0) {
        wrcu_be_detected = SYS_BIG_ENDIAN;
    }
    return (boolean)wrcu_be_detected;
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

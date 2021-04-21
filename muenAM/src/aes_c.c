
#include "aes_c.h"

const int32_t AES_TBL_LOG[256] = {
            0x00, 0xff, 0xc8, 0x08, 0x91, 0x10, 0xd0, 0x36,
            0x5a, 0x3e, 0xd8, 0x43, 0x99, 0x77, 0xfe, 0x18,
            0x23, 0x20, 0x07, 0x70, 0xa1, 0x6c, 0x0c, 0x7f,
            0x62, 0x8b, 0x40, 0x46, 0xc7, 0x4b, 0xe0, 0x0e,
            0xeb, 0x16, 0xe8, 0xad, 0xcf, 0xcd, 0x39, 0x53,
            0x6a, 0x27, 0x35, 0x93, 0xd4, 0x4e, 0x48, 0xc3,
            0x2b, 0x79, 0x54, 0x28, 0x09, 0x78, 0x0f, 0x21,
            0x90, 0x87, 0x14, 0x2a, 0xa9, 0x9c, 0xd6, 0x74,
            0xb4, 0x7c, 0xde, 0xed, 0xb1, 0x86, 0x76, 0xa4,
            0x98, 0xe2, 0x96, 0x8f, 0x02, 0x32, 0x1c, 0xc1,
            0x33, 0xee, 0xef, 0x81, 0xfd, 0x30, 0x5c, 0x13,
            0x9d, 0x29, 0x17, 0xc4, 0x11, 0x44, 0x8c, 0x80,
            0xf3, 0x73, 0x42, 0x1e, 0x1d, 0xb5, 0xf0, 0x12,
            0xd1, 0x5b, 0x41, 0xa2, 0xd7, 0x2c, 0xe9, 0xd5,
            0x59, 0xcb, 0x50, 0xa8, 0xdc, 0xfc, 0xf2, 0x56,
            0x72, 0xa6, 0x65, 0x2f, 0x9f, 0x9b, 0x3d, 0xba,
            0x7d, 0xc2, 0x45, 0x82, 0xa7, 0x57, 0xb6, 0xa3,
            0x7a, 0x75, 0x4f, 0xae, 0x3f, 0x37, 0x6d, 0x47,
            0x61, 0xbe, 0xab, 0xd3, 0x5f, 0xb0, 0x58, 0xaf,
            0xca, 0x5e, 0xfa, 0x85, 0xe4, 0x4d, 0x8a, 0x05,
            0xfb, 0x60, 0xb7, 0x7b, 0xb8, 0x26, 0x4a, 0x67,
            0xc6, 0x1a, 0xf8, 0x69, 0x25, 0xb3, 0xdb, 0xbd,
            0x66, 0xdd, 0xf1, 0xd2, 0xdf, 0x03, 0x8d, 0x34,
            0xd9, 0x92, 0x0d, 0x63, 0x55, 0xaa, 0x49, 0xec,
            0xbc, 0x95, 0x3c, 0x84, 0x0b, 0xf5, 0xe6, 0xe7,
            0xe5, 0xac, 0x7e, 0x6e, 0xb9, 0xf9, 0xda, 0x8e,
            0x9a, 0xc9, 0x24, 0xe1, 0x0a, 0x15, 0x6b, 0x3a,
            0xa0, 0x51, 0xf4, 0xea, 0xb2, 0x97, 0x9e, 0x5d,
            0x22, 0x88, 0x94, 0xce, 0x19, 0x01, 0x71, 0x4c,
            0xa5, 0xe3, 0xc5, 0x31, 0xbb, 0xcc, 0x1f, 0x2d,
            0x3b, 0x52, 0x6f, 0xf6, 0x2e, 0x89, 0xf7, 0xc0,
            0x68, 0x1b, 0x64, 0x04, 0x06, 0xbf, 0x83, 0x38 };

const int32_t AES_TBL_ANTILOG[256] = {
            0x01, 0xe5, 0x4c, 0xb5, 0xfb, 0x9f, 0xfc, 0x12,
            0x03, 0x34, 0xd4, 0xc4, 0x16, 0xba, 0x1f, 0x36,
            0x05, 0x5c, 0x67, 0x57, 0x3a, 0xd5, 0x21, 0x5a,
            0x0f, 0xe4, 0xa9, 0xf9, 0x4e, 0x64, 0x63, 0xee,
            0x11, 0x37, 0xe0, 0x10, 0xd2, 0xac, 0xa5, 0x29,
            0x33, 0x59, 0x3b, 0x30, 0x6d, 0xef, 0xf4, 0x7b,
            0x55, 0xeb, 0x4d, 0x50, 0xb7, 0x2a, 0x07, 0x8d,
            0xff, 0x26, 0xd7, 0xf0, 0xc2, 0x7e, 0x09, 0x8c,
            0x1a, 0x6a, 0x62, 0x0b, 0x5d, 0x82, 0x1b, 0x8f,
            0x2e, 0xbe, 0xa6, 0x1d, 0xe7, 0x9d, 0x2d, 0x8a,
            0x72, 0xd9, 0xf1, 0x27, 0x32, 0xbc, 0x77, 0x85,
            0x96, 0x70, 0x08, 0x69, 0x56, 0xdf, 0x99, 0x94,
            0xa1, 0x90, 0x18, 0xbb, 0xfa, 0x7a, 0xb0, 0xa7,
            0xf8, 0xab, 0x28, 0xd6, 0x15, 0x8e, 0xcb, 0xf2,
            0x13, 0xe6, 0x78, 0x61, 0x3f, 0x89, 0x46, 0x0d,
            0x35, 0x31, 0x88, 0xa3, 0x41, 0x80, 0xca, 0x17,
            0x5f, 0x53, 0x83, 0xfe, 0xc3, 0x9b, 0x45, 0x39,
            0xe1, 0xf5, 0x9e, 0x19, 0x5e, 0xb6, 0xcf, 0x4b,
            0x38, 0x04, 0xb9, 0x2b, 0xe2, 0xc1, 0x4a, 0xdd,
            0x48, 0x0c, 0xd0, 0x7d, 0x3d, 0x58, 0xde, 0x7c,
            0xd8, 0x14, 0x6b, 0x87, 0x47, 0xe8, 0x79, 0x84,
            0x73, 0x3c, 0xbd, 0x92, 0xc9, 0x23, 0x8b, 0x97,
            0x95, 0x44, 0xdc, 0xad, 0x40, 0x65, 0x86, 0xa2,
            0xa4, 0xcc, 0x7f, 0xec, 0xc0, 0xaf, 0x91, 0xfd,
            0xf7, 0x4f, 0x81, 0x2f, 0x5b, 0xea, 0xa8, 0x1c,
            0x02, 0xd1, 0x98, 0x71, 0xed, 0x25, 0xe3, 0x24,
            0x06, 0x68, 0xb3, 0x93, 0x2c, 0x6f, 0x3e, 0x6c,
            0x0a, 0xb8, 0xce, 0xae, 0x74, 0xb1, 0x42, 0xb4,
            0x1e, 0xd3, 0x49, 0xe9, 0x9c, 0xc8, 0xc6, 0xc7,
            0x22, 0x6e, 0xdb, 0x20, 0xbf, 0x43, 0x51, 0x52,
            0x66, 0xb2, 0x76, 0x60, 0xda, 0xc5, 0xf3, 0xf6,
            0xaa, 0xcd, 0x9a, 0xa0, 0x75, 0x54, 0x0e, 0x01 };

const int32_t AES_SHIFT_ROWS_MAP[16] = { 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11 };

boolean AES_TABLESINIT = 0;

const int aes_init_common_tables(){
    if(!AES_TABLESINIT){
        int32_t i = 0;
        for(i = 0; i < 256; i++){
            AES_RCON[i] = aes_rcon(i);
            AES_SBOX[i] = aes_sbox(i);
            AES_SBOXINV[AES_SBOX[i]] = i;
        }
        AES_TABLESINIT = 1;
        return 1;
    }
    return 0;
}

const int32_t aes_rcon(int32_t in){
    if(in == 0) return 0;

    int32_t c = 1;
    while(in != 1){
        int32_t b = c & 0x80;
        c = (c << 1) & 0xFF;
        if(b == 0x80) c ^= 0x1b;
        in--;
    }

    return c;
}

const int32_t aes_sbox(int32_t in){
    uint32_t s = 0;
    if(in != 0) s = AES_TBL_ANTILOG[255 - AES_TBL_LOG[in]];
    uint32_t x = s;
    int32_t c = 0;
    for(c = 0; c < 4; c++){
        //s = s ROL 1
        s = ((s << 1) & 0xFF) | (s >> 7);
        x ^= s;
    }
    x ^= 0x63;
    //System.err.println("sbox[" + in + "] = " + Integer.toHexString(x));
    //(Dum dee dum wonder where I copied this from)
    return (int32_t)x;
}

void aes_gen_key_schedule_128(aes_key128_t* key){
    if(!key) return;
    if(key->is_init) return;
    aes_init_common_tables();

    const int bcount = AES_KEYSLOTS_128 * AES_KEYBYTES_128;

    //key_schedule = new byte[slots][kbytes];
    //FileBuffer buff = new FileBuffer(slots * kbytes);
    ubyte* buff =  (ubyte*)key->key_sched; //I'm gonna try and cheese it

    //Copy key for first set
    int i,j,k,c;
    ubyte* buffptr = buff;
    int32_t row[AES_KEYBYTES_128];
    int32_t lastrow[AES_KEYBYTES_128];
    for(i = 0; i < AES_KEYBYTES_128; i++){
        //key_schedule[0][i] = aes_key[i];
        *(buffptr++) = key->aes_key[i];
        lastrow[i] = ((int32_t)key->aes_key[i]) & 0xFF;
    }

    int32_t temp[4];
    int32_t last4[4];
    for(i = 0; i < 4; i++) last4[i] = ((int32_t)key->aes_key[(AES_KEYBYTES_128-4)+i]) & 0xFF;

    int words = AES_KEYBYTES_128 >> 2;
    for(i = 1; i < AES_KEYSLOTS_128; i++){
        //Per row
        c = 0;

        //First word
        for(k = 0; k < 4; k++) temp[k] = last4[k];

        //Do the complex thing
        //--- temp ROL 8
        int32_t msb = temp[0];
        for(k = 0; k < 3; k++) temp[k] = temp[k+1];
        temp[3] = msb;
        //--- sbox
        for(k = 0; k < 4; k++) temp[k] = AES_SBOX[temp[k]];
        //--- rcon
        temp[0] ^= AES_RCON[i];

        //Do the regular thing
        for(k = 0; k < 4; k++){
            row[c] = last4[k] = (lastrow[c] ^ temp[k]);
            c++;
        }

        //Other words
        for(j = 1; j < words; j++){
            //Per word
            for(k = 0; k < 4; k++){
                //Per byte
                temp[k] = last4[k];
                row[c] = last4[k] = (lastrow[c] ^ temp[k]);
                c++;
            }
        }

        //Copy back to key schedule and lastrow
        for(j = 0; j < AES_KEYBYTES_128; j++){
            //key_schedule[i+1][j] = (byte)row[j];
            *(buffptr++) = (ubyte)row[j];
            lastrow[i] = row[j];
        }
    }
    key->is_init = 1;

    //Restructure into rows of 16
    /*int rows = bcount >>> 4;
    key_schedule = new byte[rows][16];
    buff.setCurrentPosition(0);
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < 16; j++) key_schedule[i][j] = buff.nextByte();
    }*/
    //This ^ was for Java, shouldn't need, but keeping just in case
}

aes_key128_t* aes_gen_key_128(ubyte* rawkey){
    if(!rawkey) return NULL;

    aes_key128_t* key = (aes_key128_t*)malloc(sizeof(aes_key128_t));
    memset(key, 0, sizeof(aes_key128_t));
    memcpy(key->aes_key, rawkey, AES_KEYBYTES_128);
    aes_gen_key_schedule_128(key);

    return key;
}

void aesutil_add128(ubyte* src1, ubyte* src2, ubyte* dst){
    //Treats it BIG ENDIAN!
    if(!src1 || !src2 || !dst) return;

    uint16_t carry, v1, v2, sum = 0;
    int i,idx;
    for(i = 0; i < 16; i++){
        idx = 15-i;
        v1 = (uint16_t)(*(src1+idx));
        v2 = (uint16_t)(*(src2+idx));
        sum = v1+v2+carry;
        *(dst+idx) = (ubyte)sum;
        if(sum > 0xFF) carry = 1;
        else carry = 0;
    }
}

void aesutil_xor128(ubyte* src1, ubyte* src2, ubyte* dst){
    if(!src1 || !src2 || !dst) return;

    int i = 0;
    for(i = 0; i < 16; i++){
        *(dst+i) = *(src1+i) ^ *(src2+i);
    }
}

const int32_t aes_gmul(int32_t a, int32_t b){
    if(a == 0 || b == 0) return 0;
    int32_t s = (AES_TBL_LOG[a] + AES_TBL_LOG[b]) % 255;
    s = AES_TBL_ANTILOG[s];

    return s;
}

const int rijndael_enc(aes_key128_t* key, ubyte* src, ubyte* dst){
    if(!key || !src || !dst) return 0;

    if(!key->is_init) aes_gen_key_schedule_128(key);

    int32_t i,j,k,a[4],b[4],base;
    const int rounds = AES_ROUNDS_128;

    //Initial add key
    int kidx = 0; //Key slot index
    ubyte temp8a[AES_KEYBYTES_128], temp8b[AES_KEYBYTES_128];
    int32_t temp32a[AES_KEYBYTES_128];
    int32_t temp32b[AES_KEYBYTES_128];
    aesutil_xor128(src, &key->key_sched[(kidx++) << 4], temp8a); //Mm hope that keyslot addressing works... Might not.
    for(i = 0; i < AES_KEYBYTES_128; i++) temp32a[i] = (int32_t)temp8a[i] & 0xFF;

    //Rounds
    for(i = 0; i < rounds; i++){
        //Sub bytes
        for(j = 0; j < 16; j++) temp32a[j] = AES_SBOX[temp32a[j]];

        //Shift rows
        for(j = 0; j < 16; j++) temp32b[j] = temp32a[AES_SHIFT_ROWS_MAP[j]];

        //Mix columns
        for(j = 0; j < 4; j++){
            base = j << 2;
            for(k = 0; k < 4; k++){
                a[k] = temp32b[base+k];
                b[k] = aes_gmul(temp32b[base+k], 2);
            }

            temp32a[base+0] = b[0] ^ a[3] ^ a[2] ^ b[1] ^ a[1];
            temp32a[base+1] = b[1] ^ a[0] ^ a[3] ^ b[2] ^ a[2];
            temp32a[base+2] = b[2] ^ a[1] ^ a[0] ^ b[3] ^ a[3];
            temp32a[base+3] = b[3] ^ a[2] ^ a[1] ^ b[0] ^ a[0];
        }

        //Add round key
        //temp = xorArr(temp, key_schedule[kidx++]);
        for(j = 0; j < 16; j++) temp8a[j] = (ubyte)temp32a[j];
        aesutil_xor128(temp8a, &key->key_sched[(kidx++) << 4], temp8b);
    }

    //Final round
    //Sub bytes
    for(j = 0; j < 16; j++) temp8b[j] = (ubyte)AES_SBOX[temp8b[j]];

    //Shift rows
    for(j = 0; j < 16; j++) temp8a[j] = temp8b[AES_SHIFT_ROWS_MAP[j]];

    //Add round key
    //temp = xorArr(temp2, key_schedule[kidx++]);
    aesutil_xor128(temp8a, &key->key_sched[(kidx++) << 4], dst);

    return AES_KEYBYTES_128;
}

const int rijndael_dec(aes_key128_t* key, ubyte* src, ubyte* dst){
    if(!key || !src || !dst) return 0;

    if(!key->is_init) aes_gen_key_schedule_128(key);

    int32_t i,j,k,a[4],base;
    const int rounds = AES_ROUNDS_128;

    //Initial add key
    int kidx = rounds+1;
    ubyte temp8a[AES_KEYBYTES_128], temp8b[AES_KEYBYTES_128];
    //int[] temp = xorArr(in, key_schedule[kidx--]);
    aesutil_xor128(src, &key->key_sched[(kidx--) << 4], temp8a);

    //Rounds
    for(i = 0; i < rounds; i++){

        //Shift rows (inv)
        //for(int j = 0; j < 16; j++) temp2[SHIFT_ROWS_MAP[j]] = temp[j];
        for(j = 0; j < 16; j++) temp8b[AES_SHIFT_ROWS_MAP[j]] = temp8a[j];

        //Sub bytes (inv)
        //for(j = 0; j < 16; j++) temp[j] = sbox_inv[temp2[j]];
        for(j = 0; j < 16; j++) temp8a[j] = AES_SBOXINV[temp8b[j]];

        //Add round key
        //temp = xorArr(temp, key_schedule[kidx--]);
        aesutil_xor128(temp8a, &key->key_sched[(kidx--) << 4], temp8b);

        //Mix columns
        for(j = 0; j < 4; j++){
            base = j << 2;
            for(k = 0; k < 4; k++){
                a[k] = (int32_t)temp8a[base+k] & 0xFF;
            }

            temp8b[base+0] = (ubyte)(gmul(a[0], 14) ^ gmul(a[3], 9) ^ gmul(a[2], 13) ^ gmul(a[1], 11));
            temp8b[base+1] = (ubyte)(gmul(a[1], 14) ^ gmul(a[0], 9) ^ gmul(a[3], 13) ^ gmul(a[2], 11));
            temp8b[base+2] = (ubyte)(gmul(a[2], 14) ^ gmul(a[1], 9) ^ gmul(a[0], 13) ^ gmul(a[3], 11));
            temp8b[base+3] = (ubyte)(gmul(a[3], 14) ^ gmul(a[2], 9) ^ gmul(a[1], 13) ^ gmul(a[0], 11));
        }

    }

    //Final round
    //Shift rows (inv)
    //for(j = 0; j < 16; j++) temp2[SHIFT_ROWS_MAP[j]] = temp[j];
    for(j = 0; j < 16; j++) temp8a[AES_SHIFT_ROWS_MAP[j]] = temp8b[j];

    //Sub bytes (inv)
    //for(int j = 0; j < 16; j++) temp[j] = sbox_inv[temp2[j]];
    for(j = 0; j < 16; j++) temp8b[j] = AES_SBOXINV[temp8a[j]];

    //Add round key
   // temp = xorArr(temp, key_schedule[kidx]);
    aesutil_xor128(temp8b, &key->key_sched[kidx << 4], dst);

    return AES_KEYBYTES_128;
}

const int aes_encblock_cbc128(ubyte* src, ubyte* dst, aes_state128_t* state){

    if(!src || !dst || !state) return 0;
    ubyte input[16];
    aesutil_xor128(src, state->vec, input);
    rijndael_enc(state->key, input, dst);
    state->vec = dst;

    return 16;
}

const int aes_decblock_cbc128(ubyte* src, ubyte* dst, aes_state128_t* state){

    if(!src || !dst || !state) return 0;
    ubyte temp[16];
    rijndael_dec(state->key, src, temp);
    aesutil_xor128(temp, state->vec, dst);
    state->vec = src;
    return 16;
}

const size_t aes_enc_cbc128(ubyte* src, ubyte* dst, const size_t length, ubyte* key, ubyte* iv){

    if(length % AES_KEYBYTES_128 != 0) return 0;
    if(!src || !dst || !key) return 0;

    size_t bcount = 0;
    ubyte zeros[AES_KEYBYTES_128];
    memset(zeros, 0, AES_KEYBYTES_128);
    if(!iv) iv = zeros;

    aes_key128_t* aeskey = aes_gen_key_128(key);
    aes_state128_t state = {aeskey, iv};
    while(bcount < length){
        aes_encblock_cbc128(src, dst, &state);
        src += AES_KEYBYTES_128;
        dst += AES_KEYBYTES_128;
        bcount += AES_KEYBYTES_128;
    }

    free(aeskey);
    return bcount;
}

const size_t aes_dec_cbc128(ubyte* src, ubyte* dst, const size_t length, ubyte* key, ubyte* iv){
    if(length % AES_KEYBYTES_128 != 0) return 0;
    if(!src || !dst || !key) return 0;

    size_t bcount = 0;
    ubyte zeros[AES_KEYBYTES_128];
    memset(zeros, 0, AES_KEYBYTES_128);
    if(!iv) iv = zeros;

    aes_key128_t* aeskey = aes_gen_key_128(key);
    aes_state128_t state = {aeskey, iv};
    while(bcount < length){
        aes_decblock_cbc128(src, dst, &state);
        src += AES_KEYBYTES_128;
        dst += AES_KEYBYTES_128;
        bcount += AES_KEYBYTES_128;
    }

    free(aeskey);
    return bcount;
}

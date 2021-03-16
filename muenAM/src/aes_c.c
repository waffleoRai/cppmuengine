
#include "aes_c.h"

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
    byte* buff =  (byte*)key->key_sched; //I'm gonna try and cheese it

    //Copy key for first set
    int i,j,k,c;
    byte* buffptr = buff;
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
    for(i = 1; i < slots; i++){
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
        for(k = 0; k < 4; k++) temp[k] = sbox[temp[k]];
        //--- rcon
        temp[0] ^= rcon[i];

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
            *(buffptr++) = (byte)row[j];
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

aes_key128_t* aes_gen_key_128(byte* rawkey){
    if(!rawkey) return NULL;

    aes_key128_t* key = (aes_key128_t*)malloc(sizeof(aes_key128_t));
    memset(key, 0, sizeof(aes_key128_t));
    memcpy(key->aes_key, rawkey, AES_KEYBYTES_128);
    aes_gen_key_schedule_128(key);

    return key;
}

void aesutil_add128(byte* src1, byte* src2, byte* dst){
    //Treats it BIG ENDIAN!
    if(!src1 || !src2 || !dst) return;

    uint16_t carry, v1, v2, sum = 0;
    int i,idx;
    for(i = 0; i < 16; i++){
        idx = 15-i;
        v1 = (uint16_t)(*(src1+idx));
        v2 = (uint16_t)(*(src2+idx));
        sum = v1+v2+carry;
        *(dst+idx) = (byte)sum;
        if(sum > 0xFF) carry = 1;
        else carry = 0;
    }
}

void aesutil_xor128(byte* src1, byte* src2, byte* dst){
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

const int rijndael_enc(aes_key128_t* key, byte* src, byte* dst){
    if(!key || !src || !dst) return 0;

    if(!key->is_init) aes_gen_key_schedule_128(key);

    int32_t i,j,k,a[4],b[4],base;
    const int rounds = AES_ROUNDS_128;

    //Initial add key
    int kidx = 0; //Key slot index
    byte temp8a[AES_KEYBYTES_128], temp8b[AES_KEYBYTES_128];
    int32_t temp32a[AES_KEYBYTES_128];
    int32_t temp32b[AES_KEYBYTES_128];
    aesutil_xor128(src, &key->key_sched[kidx++][0], temp8a); //Mm hope that keyslot addressing works... Might not.
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
        for(j = 0; j < 16; j++) temp8a[j] = (byte)temp32a[j];
        aesutil_xor128(temp8a, &key->key_sched[kidx++][0], temp8b);
    }

    //Final round
    //Sub bytes
    for(j = 0; j < 16; j++) temp8b[j] = (byte)AES_SBOX[temp8b[j]];

    //Shift rows
    for(j = 0; j < 16; j++) temp8a[j] = temp8b[SHIFT_ROWS_MAP[j]];

    //Add round key
    //temp = xorArr(temp2, key_schedule[kidx++]);
    aesutil_xor128(temp8a, &key->key_sched[kidx++][0], dst);

    return AES_KEYBYTES_128;
}

const int rijndael_dec(aes_key128_t* key, byte* src, byte* dst){
    if(!key || !src || !dst) return 0;

    if(!key->is_init) aes_gen_key_schedule_128(key);

    int32_t i,j,k,a[4],base;
    const int rounds = AES_ROUNDS_128;

    //Initial add key
    int kidx = rounds+1;
    byte temp8a[AES_KEYBYTES_128], temp8b[AES_KEYBYTES_128];
    //int[] temp = xorArr(in, key_schedule[kidx--]);
    aesutil_xor128(src, &key->key_sched[kidx--][0], temp8a);

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
        aesutil_xor128(temp8a, &key->key_sched[kidx--][0], temp8b);

        //Mix columns
        for(j = 0; j < 4; j++){
            base = j << 2;
            for(k = 0; k < 4; k++){
                a[k] = (int32_t)temp8a[base+k] & 0xFF;
            }

            temp8b[base+0] = (byte)(gmul(a[0], 14) ^ gmul(a[3], 9) ^ gmul(a[2], 13) ^ gmul(a[1], 11));
            temp8b[base+1] = (byte)(gmul(a[1], 14) ^ gmul(a[0], 9) ^ gmul(a[3], 13) ^ gmul(a[2], 11));
            temp8b[base+2] = (byte)(gmul(a[2], 14) ^ gmul(a[1], 9) ^ gmul(a[0], 13) ^ gmul(a[3], 11));
            temp8b[base+3] = (byte)(gmul(a[3], 14) ^ gmul(a[2], 9) ^ gmul(a[1], 13) ^ gmul(a[0], 11));
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
    aesutil_xor128(temp8b, &key->key_sched[kidx][0], dst);

    return AES_KEYBYTES_128;
}

const int aes_encblock_cbc128(byte* src, byte* dst, aes_state128_t* state){

    if(!src || !dst || !state) return 0;
    byte input[16];
    aesutil_xor128(src, state->vec, input);
    rijndael_enc(state->key, input, dst);
    state->vec = dst;

    return 16;
}

const int aes_decblock_cbc128(byte* src, byte* dst, aes_state128_t* state){

    if(!src || !dst || !state) return 0;
    byte temp[16];
    rijndael_dec(state->key, src, temp);
    aesutil_xor128(temp, state->vec, dst);
    state->vec = src;
    return 16;
}

const size_t aes_enc_cbc128(byte* src, byte* dst, const size_t length, byte* key, byte* iv){

    if(length % AES_KEYBYTES_128 != 0) return 0;
    if(!src || !dst || !key) return 0;

    size_t bcount = 0;
    byte zeros[AES_KEYBYTES_128];
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

const size_t aes_dec_cbc128(byte* src, byte* dst, const size_t length, byte* key, byte* iv){
    if(length % AES_KEYBYTES_128 != 0) return 0;
    if(!src || !dst || !key) return 0;

    size_t bcount = 0;
    byte zeros[AES_KEYBYTES_128];
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

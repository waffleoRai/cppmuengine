#include "muenaes.h"

namespace waffleoRai_muengine{

const bool MuenDecryptStream::nextBlock(){
    //First, populate input by pulling from source
    //(return false if < 16 remain!)
    int i = 0;
    for(i = 0; i < 16; i++){
        if(input.streamEnd()) return false;
        *(ibuff+i) = input.nextByte();
    }

    //Then decrypt
    aes_decblock_cbc128(ibuff, obuff, &aes_state);

    //Update pointers and positions
    opos = 0;
    ubyte* t = ibuff;
    ibuff = vec;
    vec = t;

    return true;
}

const ubyte MuenDecryptStream::nextByte(){
    if(opos >= 16){
        if(!nextBlock()) return 0;
    }

    return *(obuff + opos++);
}

const size_t MuenDecryptStream::remaining() const{
    return input.remaining() + (16-opos);
}

const bool MuenDecryptStream::streamEnd() const{
    return(remaining() <= 0);
}

void MuenDecryptStream::open(){
    if(!input.isOpen()) input.open();

    nextBlock();
    is_open = true;
}

void MuenDecryptStream::close(){
    if(!is_open) return;
    if(delsrc_on_close) delete &input;
    //Shouldn't need to free anything else...
    is_open = false;
}

const bool MuenEncryptStream::flushOutput(){

    aes_decblock_cbc128(ibuff, obuff, &aes_state);

    output.addBytes(obuff, 16);
    opos = 0;
    ubyte* t = obuff;
    obuff = vec;
    vec = t;

    return true;
}

const bool MuenEncryptStream::addByte(ubyte b){

    if(opos >= 16){
       if (!flushOutput()) return false;
    }
    *(ibuff + opos++) = b;
    return true;
}

const bool MuenEncryptStream::addBytes(const ubyte* data, const size_t datlen){

    //To be on the safe side with block alignment, this WILL copy bytes into buffer...
    size_t ct = 0;
    const ubyte* ptr = data;
    for(ct = 0; ct < datlen; ct++){
       if(!addByte(*ptr++)) return false;
    }

    return true;
}

void MuenEncryptStream::open(){
    if(!output.isOpen()) output.open();
    is_open = true;
}

void MuenEncryptStream::close(){
    if(!is_open) return;
    //If there is an incomplete block, zero pad and encrypt before closing
    if(opos > 0 && opos < 16){
        while(opos < 16) addByte(0);
        flushOutput();
    }

    if(deltrg_on_close) delete &output;
    is_open = false;
}

}

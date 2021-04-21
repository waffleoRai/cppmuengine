#ifndef MUENAES_H_INCLUDED
#define MUENAES_H_INCLUDED

//For encryption/decryption streams

#include "FileStreamer.h"
#include "aes_c.h"

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

class WRMUENAM_DLL_API MuenDecryptStream:public DataStreamerSource{

private:
    DataStreamerSource& input;
    ubyte buffer[48];

    ubyte* vec;
    ubyte* ibuff;
    ubyte* obuff;
    int opos;

    aes_state128_t aes_state;

    bool delsrc_on_close;
    bool is_open;

    const bool nextBlock();

public:
    MuenDecryptStream(DataStreamerSource& src, aes_key128_t* key, ubyte* init_vec):input(src),opos(0), delsrc_on_close(false),is_open(false){
        aes_state.key = key;
        memcpy(buffer, init_vec, 16);
        aes_state.vec = buffer;

        vec = buffer;
        ibuff = buffer+16;
        obuff = buffer+32;

        memset(ibuff, 0, 16);
        memset(obuff, 0, 16);
    }

    const ubyte nextByte() override;
	const size_t remaining() const override;
	const bool streamEnd() const override;

	void open() override;
	void close() override;
	const bool isOpen() const override{return is_open;}

	const bool deleteSourceOnClose() const{return delsrc_on_close;}
	void setDeleteSourceOnClose(bool flag){delsrc_on_close = flag;}

    virtual ~MuenDecryptStream(){close();}

};

class WRMUENAM_DLL_API MuenEncryptStream:public DataOutputTarget{

private:
    DataOutputTarget& output;
    ubyte buffer[48];

    ubyte* vec;
    ubyte* ibuff;
    ubyte* obuff;
    int opos;

    aes_state128_t aes_state;

    bool deltrg_on_close;
    bool is_open;

    const bool flushOutput();

public:
    MuenEncryptStream(DataOutputTarget& trg, aes_key128_t* key, ubyte* init_vec):output(trg),opos(0), deltrg_on_close(false),is_open(false){
        aes_state.key = key;
        memcpy(buffer, init_vec, 16);
        aes_state.vec = buffer;

        vec = buffer;
        ibuff = buffer+16;
        obuff = buffer+32;

        memset(ibuff, 0, 16);
        memset(obuff, 0, 16);
    }

    const bool addByte(ubyte b) override;
	const bool addBytes(const ubyte* data, const size_t datlen) override;
	const bool isOpen() const override{return is_open;}

	void open() override;
    void close() override;

	const bool deleteTargetOnClose(){return deltrg_on_close;}
	void setDeleteTargetOnClose(bool flag){deltrg_on_close = flag;}

    virtual ~MuenEncryptStream(){close();}

};

}

#endif // MUENAES_H_INCLUDED

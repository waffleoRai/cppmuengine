#ifndef MUENZIP_H_INCLUDED
#define MUENZIP_H_INCLUDED

//For compression/decompression streams
#include <string.h>

#include "FileStreamer.h"
#include "zlib.h"

#define MUENZIP_DEFLATE_LEVEL 7
#define MUENZIP_NOCOPY_THRESH 1028

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

class WRMUENAM_DLL_API MuenUnzipStream:public DataStreamerSource{

private:
    DataStreamerSource& input;
    z_stream zstr;

    size_t decomp_sz;
    size_t output_ct;

    const size_t buffer_sz;
    ubyte* ibuffer;
    ubyte* ibuff_p0;
    ubyte* ibuff_p1;

    ubyte* obuffer;
    ubyte* obuff_p0;
    ubyte* obuff_p1;

    bool delsrc_on_close;
    bool is_open;
    bool z_end_flag;
    int zerr;

    void fillInputBuffer();
    void unzipNextBlock();

public:
    MuenUnzipStream(DataStreamerSource& src, const size_t buffer_size, const size_t decomp_size):input(src),decomp_sz(decomp_size),output_ct(0),
        buffer_sz(buffer_size),ibuffer(nullptr),ibuff_p0(nullptr),ibuff_p1(nullptr),obuffer(nullptr),obuff_p0(nullptr),obuff_p1(nullptr),
        delsrc_on_close(false),is_open(false),z_end_flag(false),zerr(Z_OK),zstr(){}

    const ubyte nextByte() override;
	const size_t remaining() const override;
	const bool streamEnd() const override;

	void open() override;
	void close() override;
	const bool isOpen() const override{return is_open;}

	const bool deleteSourceOnClose() const{return delsrc_on_close;}
	void setDeleteSourceOnClose(bool flag){delsrc_on_close = flag;}

	const int getZError() const{return zerr;}

    virtual ~MuenUnzipStream(){close();}

};

class WRMUENAM_DLL_API MuenZipStream: public DataOutputTarget{

private:
    DataOutputTarget& output;
    z_stream zstr;

    const size_t buffer_sz;
    ubyte* ibuffer;
    ubyte* ibuff_p0;
    ubyte* ibuff_p1;

    ubyte* obuffer;
    ubyte* obuff_p0;
    ubyte* obuff_p1;

    bool deltrg_on_close;
    bool is_open;
    bool opt_txt; //Tell zlib to optimize for text instead of binary data
    int zerr;

    const bool flushOutput();
    const bool zipNextBlock();

public:

    MuenZipStream(DataOutputTarget& trg, const size_t buffer_size):output(trg),buffer_sz(buffer_size),deltrg_on_close(false),
        is_open(false),ibuffer(nullptr),ibuff_p0(nullptr),ibuff_p1(nullptr),obuffer(nullptr),obuff_p0(nullptr),obuff_p1(nullptr),
        opt_txt(false),zerr(Z_OK),zstr(){}

    const bool addByte(ubyte b) override;
	const bool addBytes(const ubyte* data, const size_t datlen) override;
	const bool isOpen() const override{return is_open;}

	void open() override;
    void close() override;

	const bool deleteTargetOnClose(){return deltrg_on_close;}
	void setDeleteTargetOnClose(bool flag){deltrg_on_close = flag;}
	void setTextInputFlag(bool flag){opt_txt = flag;}

	const int getZError() const{return zerr;}

    virtual ~MuenZipStream(){close();}

};

}

#endif // MUENZIP_H_INCLUDED

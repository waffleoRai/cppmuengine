
#include "muenzip.h"

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

void MuenUnzipStream::fillInputBuffer(){
    while(ibuff_p1 < obuffer && !input.streamEnd()){
        *(ibuff_p1++) = input.nextByte();
    }
}

void MuenUnzipStream::unzipNextBlock(){
    //obuffer will be overwritten!
    //Update input ptr/ input available
    if(ibuff_p0 == ibuff_p1){
        //Refill the buffer
        ibuff_p0 = ibuff_p1 = ibuffer;
        fillInputBuffer();
    }

    zstr.next_in = (Bytef*)ibuff_p0;
    zstr.avail_in = (uInt)(ibuff_p1 - ibuff_p0);
    uLong ict = zstr.total_in;
    uLong oct = zstr.total_out;

    //Unzip
    int result = inflate(&zstr, Z_SYNC_FLUSH);
    if(result == Z_STREAM_END) z_end_flag = true;
    else if(result != Z_OK){
        zerr = result;
        return;
    }

    //Update positions
    ibuff_p0 += (zstr.total_in - ict);
    obuff_p0 = obuffer;
    obuff_p1 = obuff_p0 + (zstr.total_out - oct);
}

const ubyte MuenUnzipStream::nextByte(){
    //if(decomp_sz - output_ct <= 0) return 0;
    if(obuff_p0 < obuff_p1){
        output_ct++;
        return *(obuff_p0++);
    }
    if(z_end_flag) return 0;
    if(decomp_sz - output_ct <= 0) return 0;
    unzipNextBlock();
    if(obuff_p0 >= obuff_p1) return 0;

    output_ct++;
    return *(obuff_p0++);
}

const u64 MuenUnzipStream::remaining() const{
    return decomp_sz - output_ct;
}

const bool MuenUnzipStream::streamEnd() const{
    //Already output expected amount.
    if(decomp_sz - output_ct <= 0) return true;

    //Input is dry (premature termination)
    if(obuff_p0 >= obuff_p1 && ibuff_p0 >= ibuff_p1 && input.remaining() <= 0) return true;

    return false;
}

void MuenUnzipStream::open(){
    if(!input.isOpen()) input.open();

    //Clear the zstr
    memset(&zstr, 0, sizeof(z_stream));

    //Allocate buffers
    //Gives 25% of requested buffer space to input and 75% to output
    ibuffer = (ubyte*)malloc(buffer_sz);
    ibuff_p0 = ibuff_p1 = ibuffer;
    obuffer = ibuffer + (buffer_sz >> 2);
    obuff_p0 = obuff_p1 = obuffer;

    //Set applicable zstr fields
    zstr.next_in = (Bytef*)ibuffer;
    zstr.next_out = (Bytef*)obuffer;
    zstr.avail_in = (uInt)(obuffer - ibuffer);
    zstr.avail_out = (uInt)buffer_sz - zstr.avail_in;

    //This is redundant given the memset, but writing here for formality and later modification.
    zstr.zalloc = Z_NULL; //Might update these if needed for thread-safety later on
    zstr.zfree = Z_NULL;
    zstr.opaque = Z_NULL;

    //Init zstr!
    int result = inflateInit(&zstr);

    if(result != Z_OK){
        //Clear out buffers and return
        zerr = result;
        is_open = false;
        free(ibuffer);
    }

    fillInputBuffer();
    is_open = true;
}

void MuenUnzipStream::close(){
    if(!is_open) return;

    //Close the zstr
    int result = inflateEnd(&zstr);
    if(result != Z_OK) zerr = result;

    //Free internal buffers
    free(ibuffer);
    ibuff_p0 = ibuff_p1 = ibuffer = nullptr;
    obuff_p0 = obuff_p1 = obuffer = nullptr;

    //Delete source if requested
    if(delsrc_on_close) delete &input;

    //Update flag
    is_open = false;
}

const bool MuenZipStream::flushOutput(){
    size_t amt = (size_t)(obuff_p1 - obuff_p0);
    if(amt <= 0) return true; //Nothin to do

    const bool res = output.addBytes(obuff_p0, amt);
    obuff_p1 = obuff_p0 = obuffer;

    return res;
}

const bool MuenZipStream::zipNextBlock(){
    //Will zip until the input buffer is empty, even if it can't do it in one go.
    if(ibuff_p0 >= ibuff_p1) return true; //Didn't do anything, but no error either

    uLong tti = zstr.total_in;
    uLong tto = zstr.total_out;

    while(ibuff_p0 < ibuff_p1){
        zstr.next_in = (Bytef*)ibuff_p0;
        zstr.avail_in = (uInt)(ibuff_p1 - ibuff_p0);
        zstr.next_out = (Bytef*)obuff_p0;
        zstr.avail_out = (uInt)((obuffer + (buffer_sz >> 1)) - obuff_p0);
        obuff_p1 = obuff_p0;

        int res = deflate(&zstr, Z_SYNC_FLUSH);
        if(res != Z_OK){
            zerr = res;
            return false;
        }

        uLong ict = zstr.total_in - tti;
        uLong oct = zstr.total_out - tto;
        ibuff_p0 += ict;
        obuff_p1 += oct;

        flushOutput();

        tti = zstr.total_in;
        tto = zstr.total_out;
    }

    return true;
}

const bool MuenZipStream::addByte(ubyte b){
    //Should flush AFTER byte filling buffer is added
    //I don't think it should be able to bleed into the outbuffer if only addByte is called
    *(ibuff_p1++) = b;
    if(ibuff_p1 >= obuffer){
        return zipNextBlock();
    }

    return true;
}

const bool MuenZipStream::addBytes(const ubyte* data, const size_t datlen){
    //If sufficiently long, don't bother copying to input buffer, hand data ptr to zstr
    Bytef* dpos = (Bytef*)data;
    size_t remaining = (size_t)datlen;
    const size_t ospace = buffer_sz >> 1;
    size_t cpamt = 0;
    size_t aamt = 0;
    uLong tti = zstr.total_in;
    uLong tto = zstr.total_out;
    int res = Z_OK;
    if(datlen >= MUENZIP_NOCOPY_THRESH){
        while(remaining > 0){
            zstr.next_in = dpos;
            cpamt = remaining>ospace?ospace:remaining;
            zstr.avail_in = (uInt)cpamt;

            res = deflate(&zstr, Z_SYNC_FLUSH);
            if(res != Z_OK){
                zerr = res;
                return false;
            }

            obuff_p1 += zstr.total_out - tto;
            flushOutput();

            aamt = zstr.total_in - tti;
            dpos += aamt;
            remaining -= aamt;

            tti = zstr.total_in;
            tto = zstr.total_out;
        }

        //Clean up zstr values before returning
        zstr.next_in = (Bytef*)ibuff_p0;
        zstr.avail_in = (uInt)(ibuff_p1 - ibuff_p0);
        return true;
    }
    else{
        while(remaining > 0){
            if(!addByte(*(dpos++))) return false;
            remaining--;
        }
        return true;
    }
}

void MuenZipStream::open(){
    if(!output.isOpen()) output.open();

    //Clear the zstr
    memset(&zstr, 0, sizeof(z_stream));

    //Allocate buffers
    //Gives 50% of requested buffer space to input and 50% to output
    ibuffer = (ubyte*)malloc(buffer_sz);
    ibuff_p0 = ibuff_p1 = ibuffer;
    obuffer = ibuffer + (buffer_sz >> 1);
    obuff_p0 = obuff_p1 = obuffer;

    //Set applicable zstr fields
    zstr.next_in = (Bytef*)ibuffer;
    zstr.next_out = (Bytef*)obuffer;
    zstr.avail_in = (uInt)(obuffer - ibuffer);
    zstr.avail_out = (uInt)buffer_sz - zstr.avail_in;
    if(opt_txt) zstr.data_type = Z_TEXT;
    else zstr.data_type = Z_BINARY;

    //This is redundant given the memset, but writing here for formality and later modification.
    zstr.zalloc = Z_NULL; //Might update these if needed for thread-safety later on
    zstr.zfree = Z_NULL;
    zstr.opaque = Z_NULL;

    //Init zstr!
    int result = deflateInit(&zstr, MUENZIP_DEFLATE_LEVEL);

    if(result != Z_OK){
        //Clear out buffers and return
        zerr = result;
        is_open = false;
        free(ibuffer);
    }

    is_open = true;
}

void MuenZipStream::close(){
    if(!is_open) return;
    //Write anything unwritten
    zipNextBlock();

    //Close the zstr
    int result = deflateEnd(&zstr);
    if(result != Z_OK) zerr = result;

    //Free internal buffers
    free(ibuffer);
    ibuff_p0 = ibuff_p1 = ibuffer = nullptr;
    obuff_p0 = obuff_p1 = obuffer = nullptr;

    //Delete source if requested
    if(deltrg_on_close) delete &output;

    //Update flag
    is_open = false;

}

}

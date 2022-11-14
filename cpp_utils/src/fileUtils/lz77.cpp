
#include "lz77.h"

namespace waffleoRai_Utils{

/*--- LZ77 Decompressor ---*/

const uint LZ77Decompressor::bufferBlock(){
    if(streamEnd()) return false;

    //Determine what to do next.
    if(!processNextCommand()) return false;

    //Copy plaintext...
    uint count = 0;
    for(uint i = 0; i < read_plain; i++){
        ubyte b = src.nextByte();
        if(rwin.put(b)) count++;
        else return count;

        if(bwin.isFull()) bwin.pop();
        bwin.put(b);
    }

    //Backcopy...
    if(backread_count > 0){
        if (!bwin.setRandomAccessPositionBack(backread_off)) {
            printf("DEBUG -- Uh oh, we have a back window problem... Read count: 0x%llx\n", read_count);
            return count;
        } 
        for(uint i = 0; i < backread_count; i++){
            ubyte b = bwin.getNextRAByte();

            if(rwin.put(b)) count++;
            else {
                printf("DEBUG -- Uh oh, we have a read window problem... Read count: 0x%llx\n", read_count);
                return count;
            }

            if(bwin.isFull()) bwin.pop();
            bwin.put(b);
        }
    }

    return count;
}

const int LZ77Decompressor::get() {
    if (rwin.isEmpty() && !bufferBlock()) return EOF;
    return static_cast<int>(nextByte()) & 0xff;
}

const ubyte LZ77Decompressor::nextByte(){
    //Check if there are bytes left
    //if(streamEnd()) return 0xFF;

    //Check read buffer for bytes
    //If read buffer is empty, do next block
    if(rwin.isEmpty() && !bufferBlock()) return 0xFF;

    read_count++;
    return rwin.pop();
}

const bool LZ77Decompressor::remainingToEndKnown() const {
    return (decomp_size > 0);
}

const size_t LZ77Decompressor::remaining() const{
    if(decomp_size < 0) return decomp_size;
    return decomp_size - read_count;
}

const bool LZ77Decompressor::streamEnd() const{
    return ((src.streamEnd() && rwin.isEmpty()) || (decomp_size > 0 && read_count >= decomp_size));
}

void LZ77Decompressor::close(){
    if(closed) return;
    src.close();
    if(flag_free_on_close) delete &src;
    closed = true;
}

/*--- LZ77 Compressor ---*/

void LZ77Compressor::processNextByte() {
    u32 fpos = 0;
    u32 bpos = 0;
    u32 bsize, fsize;
    int spos = 0;
    int i = 0;
    bool usef = false;
    ubyte bytef = 0, byter = 0;
    ubyte first_byte = 0;
    uint32_t minrun = global_streak_min;
    vector<LZCompRule>::const_iterator vitr = streak_mins.cbegin();
    vector<LZCompRule>::const_iterator vend = streak_mins.cend();

    streak_count = 0;
    streak_off = 0;
    bsize = bwin.getCurrentSize();
    fsize = fwin.getCurrentSize();

 /*   printf("fwin peek: ");
    for (int i = 0; i < 16; i++) printf("%02x ", fwin.getByteAt(i));
    printf("\nbwin back: ");
    int readback = bsize < 16 ? bsize : 16;
    bwin.setRandomAccessPositionBack(readback);
    for (int i = 0; i < readback; i++) printf("%02x ", bwin.getNextRAByte());
    printf("\n");
    */

    first_byte = fwin.getByteAt(0);
    for (bpos = 0; bpos < bsize; bpos++) {

        if (bwin.getByteFromBack(bpos+1) != first_byte) continue;

        spos = bpos - 1; usef = false; fpos = 1;
        fwin.setRandomAccessPosition(1);
        if (spos < 0) {
            usef = true;
            spos = 0;
        }
        else bwin.setRandomAccessPositionBack(bpos);
        do {
            if (fpos >= fsize) break;
            if (usef && (spos >= fsize)) break;

            bytef = fwin.getNextRAByte(); fpos++;
            if (usef) {
                byter = fwin.getByteAt(spos++);
            }
            else {
                byter = bwin.getNextRAByte(); spos--;
                if (spos < 0) {
                    usef = true;
                    spos = 0;
                }
            }
        } while (bytef == byter);

        //fpos represents how many were read, which may be one more than were matched.
        if (bytef != byter) fpos--; //If it broke from mismatch, not because end was reached.
        if (fpos < minrun) continue;

        //Update minimum run size...
        while (vitr != vend && bpos >= vitr->max_offset) {
            minrun = vitr->max_run;
            vitr++;
        }
        if (fpos < minrun) continue;
        if ((fpos > streak_count)) {
            streak_count = fpos;
            streak_off = bpos;
        }
    }
    //printf("\n");
}

const int LZ77Compressor::get() {
    if (write_buffer.isEmpty() && fwin.isEmpty() && src.streamEnd()) return EOF;
    return static_cast<int>(nextByte()) & 0xff;
}

const ubyte LZ77Compressor::nextByte() {
    int read = 0;
    int i = 0;

    if (!write_buffer.isEmpty()) {
        output_bytes++;
        return write_buffer.pop();
    }

    while (!fwin.isFull() && !src.streamEnd()) {
        /*ubyte b = src.nextByte();
        printf("Read byte into forward window: %02x | Position = %llx\n", b, input_bytes);
        fwin.put(b);*/

        fwin.put(src.nextByte());
        input_bytes++;
    }

    if (fwin.isEmpty()) return 0;
    processNextByte();
    read = encodeToWriteBuffer();

    for (i = 0; i < read; i++) {
        if (bwin.isFull()) bwin.pop();
        bwin.put(fwin.pop());
    }

    output_bytes++;
    return write_buffer.pop();
}

const size_t LZ77Compressor::remaining() const {
    //HMMMMM that's a problem. Might just have to return what's in the write buffer.
    return write_buffer.getCurrentSize();
}

const bool LZ77Compressor::streamEnd() const {
    //printf("%d %d %d\n", write_buffer.getCurrentSize(), fwin.getCurrentSize(), src.streamEnd());
    return write_buffer.isEmpty() && fwin.isEmpty() && src.streamEnd();
}

void LZ77Compressor::close() {
    if (closed) return;
    src.close();
    if (flag_free_on_close) delete& src;
    closed = true;
}

}

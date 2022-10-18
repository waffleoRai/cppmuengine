
#include "lz77.h"

namespace waffleoRai_Utils{

/*--- Array Window ---*/

const ubyte ArrayWindow::pop(){
    if(used_size <= 0) return 0;
    ubyte b = *(read_pos++);
    if(read_pos >= buffer_end) read_pos = buffer_start;
    used_size--;
    return b;
}

const ubyte ArrayWindow::peek() const{
    if(used_size <= 0) return 0;
    return *(read_pos);
}

const bool ArrayWindow::push(ubyte b){
    if(isFull()) return false;
    if(--read_pos < buffer_start) read_pos = buffer_end-1;
    *(read_pos) = b;
    used_size++;
    return true;
}

const bool ArrayWindow::put(ubyte b){
    if(isFull()) return false;
    if(write_pos >= buffer_end) write_pos = buffer_start;
    *(write_pos++) = b;
    used_size++;
    return true;
}

const bool ArrayWindow::isEmpty() const{
    return (used_size == 0);
}

const bool ArrayWindow::isFull() const{
    return (used_size >= (getCapacity()));
}

const void ArrayWindow::clear(){
    used_size = 0;
    write_pos = buffer_start;
    read_pos = buffer_start;
}

const uint ArrayWindow::removeFromFront(uint amt){
    //"Pops" the number of specified bytes from the front
    if(amt>used_size) amt = used_size;
    ubyte* npos = read_pos + amt;
    if(npos >= buffer_end){
        npos = buffer_start + (npos - buffer_end);
    }
    read_pos = npos;
    used_size -= amt;
    return amt;
}

const uint ArrayWindow::putBytes(const ubyte* bytes, uint len){
    size_t rem = getAvailableSize();
    if(rem <= 0) return 0;
    if(rem < len) len = rem;

    for(uint i = 0; i < len; i++){
        if(write_pos >= buffer_end) write_pos = buffer_start;
        *(write_pos++) = *(bytes+i);
        used_size++;
    }

    return len;
}

const bool ArrayWindow::setRandomAccessPosition(uint pos){
    if(pos >= used_size) return false;
    ubyte* mpos = read_pos + pos;
    if(mpos >= buffer_end) mpos = buffer_start + (mpos - buffer_end);
    random_pos = mpos;
    return true;
}

const bool ArrayWindow::setRandomAccessPositionBack(uint pos_from_back){
    if(pos_from_back > used_size) return false;
    ubyte* mpos = write_pos - pos_from_back;
    if(mpos < buffer_start) mpos = buffer_end - (buffer_start - mpos);
    random_pos = mpos;
    return true;
}

const ubyte ArrayWindow::getNextRAByte(){
    //DOES NOT CHECK to see if you are out of bounds!
    //MAY RETURN GARBAGE!!!!
    ubyte b = *(random_pos++);
    if(random_pos >= buffer_end) random_pos = buffer_start;
    return b;
}

const ubyte ArrayWindow::getByteAt(uint pos) {
    if (pos >= used_size) return 0;
    ubyte* mpos = read_pos + pos;
    if (mpos >= buffer_end) mpos = buffer_start + (mpos - buffer_end);
    return *mpos;
}

const size_t ArrayWindow::getCurrentSize() const{
    return used_size;
}

const size_t ArrayWindow::getCapacity() const{
    return buffer_end - buffer_start;
}

const size_t ArrayWindow::getAvailableSize() const{
    return getCapacity() - getCurrentSize();
}

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

const ubyte LZ77Decompressor::nextByte(){
    //Check if there are bytes left
    //if(streamEnd()) return 0xFF;

    //Check read buffer for bytes
    //If read buffer is empty, do next block
    if(rwin.isEmpty() && !bufferBlock()) return 0xFF;

    read_count++;
    return rwin.pop();
}

const u64 LZ77Decompressor::remaining() const{
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
    bool usef = false;
    ubyte bytef = 0, byter = 0;

    streak_count = 0;
    streak_off = 0;
    bsize = bwin.getCurrentSize();
    fsize = fwin.getCurrentSize();
    
    for (bpos = 0; bpos < bsize; bpos++) {
        bwin.setRandomAccessPositionBack(bpos);
        fwin.setRandomAccessPosition(0);
        spos = bpos; usef = false;

        //How long a streak can you get from here?
        //(Don't forget that it should continue to the forward window if back ends...)
        usef = false;
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
        if (fpos > streak_count) {
            streak_count = fpos;
            streak_off = bpos;
        }
    }
}

const ubyte LZ77Compressor::nextByte() {
    int read = 0;
    int i = 0;

    if (!write_buffer.isEmpty()) {
        output_bytes++;
        return write_buffer.pop();
    }

    while (!fwin.isFull() && !src.streamEnd()) {
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
    return write_buffer.isEmpty() && fwin.isEmpty() && src.streamEnd();
}

void LZ77Compressor::close() {
    if (closed) return;
    src.close();
    if (flag_free_on_close) delete& src;
    closed = true;
}

}

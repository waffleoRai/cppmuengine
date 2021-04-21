
#include "lz77.h"

using namespace std;

namespace waffleoRai_Utils
{

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
    if(pos_from_back >= used_size) return false;
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
        if(!bwin.setRandomAccessPositionBack(backread_off)) return count;
        for(uint i = 0; i < backread_count; i++){
            ubyte b = bwin.getNextRAByte();

            if(rwin.put(b)) count++;
            else return count;

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

}

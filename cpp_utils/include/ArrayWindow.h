#ifndef ARRAYWIN_H_INCLUDED
#define ARRAYWIN_H_INCLUDED

#include "wr_cpp_utils.h"

namespace waffleoRai_Utils
{
    class WRCU_DLL_API ArrayWindow {

    private:
        ubyte* buffer_start; //Used for alloc
        ubyte* buffer_end; //Just used as pointer to save time adding

        ubyte* read_pos;
        ubyte* write_pos;
        u32 used_size;

        ubyte* random_pos; //For random access

    public:
        ArrayWindow(size_t alloc) :used_size(0) {
            buffer_start = (ubyte*)malloc(alloc);
            buffer_end = buffer_start + alloc;

            read_pos = buffer_start;
            write_pos = buffer_start;
            random_pos = buffer_start;
        }

        const ubyte pop() {
            if (used_size <= 0) return 0xff;
            ubyte b = *(read_pos++);
            if (read_pos >= buffer_end) read_pos = buffer_start;
            used_size--;
            return b;
        }

        const ubyte peek() const {
            if (used_size <= 0) return 0;
            return *(read_pos);
        }

        const bool push(ubyte b) {
            if (isFull()) return false;
            if (--read_pos < buffer_start) read_pos = buffer_end - 1;
            *(read_pos) = b;
            used_size++;
            return true;
        }

        const bool put(ubyte b) {
            if (isFull()) return false;
            if (write_pos >= buffer_end) write_pos = buffer_start;
            *(write_pos++) = b;
            used_size++;
            return true;
        }

        const ubyte peekLast() const {
            if (used_size <= 0) return 0;
            return write_pos <= buffer_start ? *(buffer_end - 1) : *(write_pos - 1);
        }

        const bool isEmpty() const {
            return (used_size == 0);
        }

        const bool isFull() const {
            return (used_size >= (buffer_end - buffer_start));
        }

        const void clear() {
            used_size = 0;
            write_pos = buffer_start;
            read_pos = buffer_start;
        }

        const uint removeFromFront(uint amt) {
            //"Pops" the number of specified bytes from the front
            if (amt > used_size) amt = used_size;
            ubyte* npos = read_pos + amt;
            if (npos >= buffer_end) {
                npos = buffer_start + (npos - buffer_end);
            }
            read_pos = npos;
            used_size -= amt;
            return amt;
        }

        const uint putBytes(const ubyte* bytes, uint len) {
            size_t rem = getAvailableSize();
            if (rem <= 0) return 0;
            if (rem < len) len = rem;

            for (uint i = 0; i < len; i++) {
                if (write_pos >= buffer_end) write_pos = buffer_start;
                *(write_pos++) = *(bytes + i);
                used_size++;
            }

            return len;
        }

        const bool setRandomAccessPosition(uint pos) {
            if (pos >= used_size) return false;
            ubyte* mpos = read_pos + pos;
            if (mpos >= buffer_end) mpos = buffer_start + (mpos - buffer_end);
            random_pos = mpos;
            return true;
        }

        const bool setRandomAccessPositionBack(uint pos_from_back) {
            if (pos_from_back > used_size) return false;
            ubyte* mpos = write_pos - pos_from_back;
            if (mpos < buffer_start) mpos = buffer_end - (buffer_start - mpos);
            random_pos = mpos;
            return true;
        }

        const ubyte getNextRAByte() {
            //DOES NOT CHECK to see if you are out of bounds!
            //MAY RETURN GARBAGE!!!!
            ubyte b = *(random_pos++);
            if (random_pos >= buffer_end) random_pos = buffer_start;
            return b;
        }

        const ubyte getByteAt(uint pos) {
            //if (pos >= used_size) return 0;
            ubyte* mpos = read_pos + pos;
            if (mpos >= buffer_end) mpos = buffer_start + (mpos - buffer_end);
            return *mpos;
        }

        const ubyte getByteFromBack(uint pos_from_back) {
            //if (pos >= used_size) return 0;
            ubyte* mpos = write_pos - pos_from_back;
            if (mpos < buffer_start) mpos = buffer_end - (buffer_start - mpos);
            return *mpos;
        }

        const size_t getCurrentSize() const { return used_size; }
        const size_t getCapacity() const { return buffer_end - buffer_start; }
        const size_t getAvailableSize() const { return (buffer_end - buffer_start) - used_size; }

        virtual ~ArrayWindow() { free(buffer_start); }

    };
}

#endif
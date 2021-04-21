#ifndef LZ77_H_INCLUDED
#define LZ77_H_INCLUDED

#define READ_BUFFER_SIZE 512

#include "FileStreamer.h"

//using namespace std;

namespace waffleoRai_Utils
{

class ArrayWindow{

private:
    ubyte* buffer_start; //Used for alloc
    ubyte* buffer_end; //Just used as pointer to save time adding

    ubyte* read_pos;
    ubyte* write_pos;
    u32 used_size;

    ubyte* random_pos; //For random access

public:
    ArrayWindow(size_t alloc):used_size(0){
        buffer_start = (ubyte*)malloc(alloc);
        buffer_end = buffer_start + alloc;

        read_pos = buffer_start;
        write_pos = buffer_start;
        random_pos = buffer_start;
    }

    const ubyte pop();
    const ubyte peek() const;
    const bool push(ubyte b);
    const bool put(ubyte b);
    const bool isEmpty() const;
    const bool isFull() const;
    const void clear();

    const uint removeFromFront(uint amt);
    const uint putBytes(const ubyte* bytes, uint len);

    const bool setRandomAccessPosition(uint pos);
    const bool setRandomAccessPositionBack(uint pos_from_back);
    const ubyte getNextRAByte();

    const size_t getCurrentSize() const;
    const size_t getCapacity() const;
    const size_t getAvailableSize() const;

    virtual ~ArrayWindow(){free(buffer_start);}

};

class LZ77Decompressor: public DataStreamerSource{

private:
    bool flag_free_on_close = false;
	bool closed = false;

protected:
    size_t decomp_size;
    DataStreamerSource& src;

    size_t back_win_size;
    ArrayWindow bwin;

    ArrayWindow rwin; //Read buffer
    size_t front_win_size;
    size_t read_count = 0;

    //Default implementation fields
    u32 read_plain = 0;
    u32 backread_count = 0;
    u32 backread_off = 0;

    virtual const bool processNextCommand() = 0;
    virtual const uint bufferBlock();

public:
    LZ77Decompressor(DataStreamerSource& source, size_t front_window_size, size_t back_window_size):DataStreamerSource(),decomp_size(-1),src(source),
        back_win_size(back_window_size),bwin(back_window_size),rwin(front_window_size),front_win_size(front_window_size){}
    LZ77Decompressor(DataStreamerSource& source, size_t front_window_size, size_t back_window_size, size_t decompressedSize):DataStreamerSource(),
        decomp_size(decompressedSize),src(source),back_win_size(back_window_size),bwin(back_window_size),rwin(front_window_size),front_win_size(front_window_size){}

    DataStreamerSource& getSource(){return src;}

    const ubyte nextByte() override;
	const size_t remaining() const override;
	const bool streamEnd() const override;

	void setFreeOnCloseFlag(bool b){flag_free_on_close = b;}
	void open() override {}
	void close() override;

	virtual ~LZ77Decompressor(){};

};

}

#endif // LZ77_H_INCLUDED

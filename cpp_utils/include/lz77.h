#ifndef LZ77_H_INCLUDED
#define LZ77_H_INCLUDED

#define READ_BUFFER_SIZE 512
#define WRITE_BUFFER_SIZE 512

#include "ArrayWindow.h"
#include "FileStreamer.h"

using std::vector;

namespace waffleoRai_Utils
{

typedef struct WRCU_DLL_API LZCompRule {
    uint32_t max_offset = 0;
    uint32_t max_run = 0;

    LZCompRule(uint32_t off, uint32_t sz) { max_offset = off; max_run = sz; }
} LZCompRule;

class WRCU_DLL_API LZ77Decompressor: public DataStreamerSource{

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

    const int get() override;
    const ubyte nextByte() override;

    const bool remainingToEndKnown() const override;
	const size_t remaining() const override;
	const bool streamEnd() const override;

	void setFreeOnCloseFlag(bool b){flag_free_on_close = b;}
    const bool isOpen() const override { return src.isOpen(); }
	virtual void open() override {}
	void close() override;

	virtual ~LZ77Decompressor(){};

};

class WRCU_DLL_API LZ77Compressor : public DataStreamerSource {
private:
    bool flag_free_on_close = false;
    bool closed = false;

protected:
    DataStreamerSource& src;

    size_t input_bytes = 0LL;
    size_t output_bytes = 0LL;

    size_t back_win_size;
    ArrayWindow bwin;

    ArrayWindow fwin;
    size_t front_win_size;

    ArrayWindow write_buffer;

    u32 streak_count = 0;
    u32 streak_off = 0;

    u32 global_streak_min = 1;
    vector<LZCompRule> streak_mins = vector<LZCompRule>(); //TODO

    virtual void processNextByte(); //Writes to streak_count and streak_off
    virtual const int encodeToWriteBuffer() = 0; //Uses streak_count and streak_off and front window to encode next block. Returns # bytes read.

public:
    LZ77Compressor(DataStreamerSource& data_source, size_t front_window_size, size_t back_window_size):DataStreamerSource(),src(data_source),
        back_win_size(back_window_size), bwin(back_window_size), fwin(front_window_size), front_win_size(front_window_size), write_buffer(WRITE_BUFFER_SIZE){}

    DataStreamerSource& getSource() { return src; }

    const int get() override;
    const ubyte nextByte() override;

    const bool remainingToEndKnown() const override { return false; }
    const size_t remaining() const override;
    const bool streamEnd() const override;

    const size_t getInputLength() { return input_bytes; }
    const size_t getOutputLength() { return output_bytes; }

    void setFreeOnCloseFlag(bool b) { flag_free_on_close = b; }
    const bool isOpen() const override { return src.isOpen(); }
    virtual void open() override {}
    void close() override;

    virtual ~LZ77Compressor() {};
};

}

#endif // LZ77_H_INCLUDED

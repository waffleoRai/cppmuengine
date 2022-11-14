#ifndef FILESTREAMER_IN_H_INCLUDED
#define FILESTREAMER_IN_H_INCLUDED

//#include "FileStreamer.h"
#include "wr_cpp_utils.h"
#include <iostream>
#include <fstream>
#include <filesystem> //Need C++ 17 !

using std::ifstream;
using std::streampos;
using std::ofstream;
using std::filesystem::path;
using icu::UnicodeString;

namespace waffleoRai_Utils {

class WRCU_DLL_API InputException :public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	InputException(const char* source, const char* reason) :sSource(source), sReason(reason) {};
	const char* what() const throw() { return sReason; }
};

class WRCU_DLL_API DataStreamerSource
{
public:
		virtual const int get() = 0;
		virtual const ubyte nextByte() = 0;

		virtual void open() = 0;
		virtual void close() = 0;
		virtual const bool isOpen() const = 0;
		virtual const bool resetReadCounter() { return false; }

		virtual const bool remainingToEndKnown() const = 0;
		virtual const size_t remaining() const = 0;
		virtual const bool streamEnd() const = 0;

		//Does nothing by default
		virtual const bool isSeekable() const;
		virtual const streampos seek(const streampos pos);

		virtual const size_t nextBytes(ubyte* dst, const size_t len); //Default implementation just calls nextByte()

		virtual ~DataStreamerSource() {}
};

class WRCU_DLL_API FileInputStreamer :public DataStreamerSource
{

private:
	const path sFilePath;

	ifstream* oOpenStream;
	size_t read = 0;
	size_t maxsz = 0; //Field to save file size so don't have to retrieve from OS when remainingBytes() is called.

	streampos start_boundary = 0;
	streampos end_boundary = 0;

public:

	FileInputStreamer(const path& path) :sFilePath(path), oOpenStream(nullptr) {}
	FileInputStreamer(const string& ascii_path) :sFilePath(ascii_path), oOpenStream(nullptr) {}
	FileInputStreamer(const char* ascii_path) :sFilePath(ascii_path), oOpenStream(nullptr) {}
	FileInputStreamer(const char16_t* utf16_path) :sFilePath(utf16_path), oOpenStream(nullptr) {}
	FileInputStreamer(const char32_t* utf32_path) :sFilePath(utf32_path), oOpenStream(nullptr) {}

	const path& getPath() const { return sFilePath; }

	const size_t fileSize() const;
	const streampos getStartBoundary() const { return start_boundary; }
	const streampos getEndBoundary() const { return end_boundary; }

	const streampos setStartBoundary(const streampos position) {
		if (start_boundary >= 0 && start_boundary < end_boundary) {
			start_boundary = position;
		}
		return start_boundary;
	}

	const streampos setEndBoundary(const streampos position) {
		if (end_boundary <= maxsz && end_boundary > start_boundary) {
			end_boundary = position;
		}
		return end_boundary;
	}

	const bool isSeekable() const override;
	const streampos seek(const streampos pos) override;

	const bool isOpen() const override;
	const bool streamEnd() const override;
	const bool isGood() const;
	const bool isBad() const;
	const bool isFail() const;
	const bool remainingToEndKnown() const override;
	const size_t remaining() const override;
	const size_t bytesRead() const { return read; }
	const bool resetReadCounter() override { read = 0; return true; }

	void open() override;
	void open(const streampos startOffset);
	void jumpTo(const streampos offset);
	void skip(const streampos skip_amt);
	void close() override;
	const ifstream& getStreamView() const;

	const ubyte nextByte() override;
	const int get() override;
	const size_t nextBytes(ubyte* dst, const size_t len) override;

	virtual ~FileInputStreamer() { close(); }

};

class WRCU_DLL_API ifstreamer : public DataStreamerSource
{

private:
	ifstream* stream;
	size_t read = 0;
	//size_t maxsz = 0;

	streampos start_boundary = 0;
	streampos end_boundary = 0;

	bool close_source_on_close = false;
	bool destroy_source_on_close = false;

public:

	ifstreamer(ifstream& source) :stream(&source) {
		if (source.is_open()) start_boundary = source.tellg();
	}

	ifstreamer(ifstream& source, size_t size_bytes) :stream(&source) {
		if (source.is_open()) start_boundary = source.tellg();
		end_boundary = static_cast<size_t>(start_boundary) + size_bytes;
	}

	const size_t fileSize() const { return end_boundary > 0 ? static_cast<size_t>(end_boundary - start_boundary) : 0; }

	const bool isOpen() const override { return stream && stream->is_open(); }
	const bool streamEnd() const override;
	const bool isGood() const { return stream->good(); }
	const bool isBad() const { return stream->bad(); }
	const bool isFail() const { return stream->fail(); }
	const bool remainingToEndKnown() const override;
	const size_t remaining() const override;
	const size_t bytesRead() const { return read; }
	const bool resetReadCounter() override { read = 0; return true; }

	const bool isSeekable() const override;
	const streampos seek(const streampos pos) override;

	void open() override;
	void skip(const streampos skip_amt);
	void close() override;
	const ifstream& getStreamView();

	const int get() override;
	const ubyte nextByte() override;
	const size_t nextBytes(ubyte* dst, const size_t len) override;

	void setCloseStreamOnClose(const bool flag) { close_source_on_close = flag; }
	void setDestroyStreamOnClose(const bool flag) { destroy_source_on_close = flag; }

	virtual ~ifstreamer() { close(); }

};

class WRCU_DLL_API MemInputStreamer : public DataStreamerSource
{
private:
	ubyte* data;
	ubyte* data_end; //Bookkeeping, for speed.
	size_t len = 0;

	ubyte* current_pos = NULL;
	bool is_open = false; //Used to mark closure. If closed, it is assumes the referenced data is expired.

public:
	MemInputStreamer(const void* src, const size_t size) :data((ubyte*)src), len(size), data_end(NULL) { data_end = data + len; };

	const int get() override { return (!is_open || current_pos >= data_end) ? EOF : static_cast<int>(*current_pos++) & 0xff; };
	const ubyte nextByte() override { return (!is_open || current_pos >= data_end) ? 0xff : *current_pos++; };
	const size_t nextBytes(ubyte* dst, const size_t len) override;

	const size_t remaining() const override { return is_open ? static_cast<size_t>(data_end - current_pos) : 0; };
	const bool streamEnd() const override { return is_open ? (current_pos >= data_end) : true; };

	void open() override;
	void skip(const streampos skip_amt);
	void close() override { is_open = false; current_pos = NULL; data = NULL; data_end = NULL; len = 0; };

	const bool isOpen() const override { return is_open; }
	const bool resetReadCounter() override { return true; }

	const bool remainingToEndKnown() const override { return true; };
	const bool isSeekable() const override { return true; }
	const streampos seek(const streampos pos) override;

	virtual ~MemInputStreamer() { close(); }
};

}

#endif

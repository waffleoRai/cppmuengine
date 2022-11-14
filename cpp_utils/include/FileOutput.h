#ifndef FILESTREAMER_OUT_H_INCLUDED
#define FILESTREAMER_OUT_H_INCLUDED

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

class WRCU_DLL_API OutputException :public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	OutputException(const char* source, const char* reason) :sSource(source), sReason(reason) {};
	const char* what() const throw() { return sReason; }
};

class WRCU_DLL_API DataOutputTarget
{

public:
	virtual const bool addByte(const ubyte b) = 0;
	virtual const bool addBytes(const ubyte* data, const size_t datlen) = 0;
	virtual const bool isOpen() const = 0;
	virtual void open() = 0;
	virtual void close() = 0;

	virtual ~DataOutputTarget() {}

};

class WRCU_DLL_API FileOutputStreamer :public DataOutputTarget
{

private:
	const path sFilePath;

	ofstream* oOpenStream;

public:
	FileOutputStreamer(const path& path) :sFilePath(path), oOpenStream(nullptr) {}
	FileOutputStreamer(const string& ascii_path) :sFilePath(ascii_path), oOpenStream(nullptr) {}
	FileOutputStreamer(const char* ascii_path) :sFilePath(ascii_path), oOpenStream(nullptr) {}
	FileOutputStreamer(const char16_t* utf16_path) :sFilePath(utf16_path), oOpenStream(nullptr) {}
	FileOutputStreamer(const char32_t* utf32_path) :sFilePath(utf32_path), oOpenStream(nullptr) {}

	const path& getPath() const { return sFilePath; }

	const bool isOpen() const override;
	void open() override;
	void openForAppending();
	void close() override;
	const ofstream& getStreamView() const;

	const bool addByte(const ubyte value) override;
	const bool addBytes(const ubyte* data, const size_t datlen) override;

	virtual ~FileOutputStreamer() { close(); }
};

};

#endif
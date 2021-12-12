#ifndef FILESTREAMER_H_INCLUDED
#define FILESTREAMER_H_INCLUDED

#include "wr_cpp_utils.h"
#include <iostream>
#include <fstream>
#include <filesystem> //Need C++ 17 !
//#include <sys/stat.h>

//using namespace std;
using std::ifstream;
using std::streampos;
using std::ofstream;
using std::cout;
using namespace std::filesystem;
using namespace icu;

namespace waffleoRai_Utils
{

enum class WRCU_DLL_API Endianness {little_endian,big_endian};

WRCU_DLL_API const bool WRCU_CDECL sys_endian_matches(Endianness e);

class WRCU_DLL_API DataStreamerSource
{

public:
	virtual const ubyte nextByte() = 0;
	virtual const size_t remaining() const = 0;
	virtual const bool streamEnd() const = 0;

	virtual void open() = 0;
	virtual void close() = 0;
	virtual const bool isOpen() const = 0;

	virtual const size_t nextBytes(ubyte* dst, const size_t len); //Default implementation just calls nextByte()

	virtual ~DataStreamerSource(){}
};

class WRCU_DLL_API DataInputStreamer
{

private:
	Endianness eEndian;
	DataStreamerSource& iSource;
	bool flag_free_on_close = false;
	bool closed = false;

	int uconv_type = 0;
	UErrorCode uerr = U_ZERO_ERROR;
	UConverter* uconv = nullptr;

	void initconv_utf8();
	void initconv_utf16_bom();
	void initconv_utf16_ordered();
	void initconv_utf32_sysordered();

public:
	DataInputStreamer(DataStreamerSource& src, const Endianness endian):eEndian(endian),iSource(src){}

	DataStreamerSource& getSource() {return iSource;}
	const Endianness getEndianness() const{return eEndian;}
	void setEndianness(const Endianness endian){eEndian = endian;}
	virtual const size_t remaining() const{return iSource.remaining();}
	virtual const bool streamEnd() const{return iSource.streamEnd();}

	const size_t skip(streampos amt);

	const ubyte nextByte(){return iSource.nextByte();}
	const uint16_t nextUnsignedShort();
	const int16_t nextShort();
	const uint32_t nextUnsigned24();
	const int32_t next24();
	const uint32_t nextUnsignedInt();
	const int32_t nextInt();
	const uint64_t nextUnsignedLong();
	const int64_t nextLong();

	const size_t nextBytes(ubyte* dst, const size_t len) { return iSource.nextBytes(dst, len); }

	void setFreeOnCloseFlag(bool b){flag_free_on_close = b;}

	//String reading
	const size_t readASCIIString(string& dst, const size_t sz_bytes);
	const size_t readASCIIString(UnicodeString& dst, const size_t sz_bytes);
	const size_t readASCIIString(char16_t* dst, const size_t sz_bytes);
	const size_t readASCIIString(char32_t* dst, const size_t sz_bytes);

	const size_t readUTF8String(UnicodeString& dst, const size_t sz_bytes);
	const size_t readUTF8String(char16_t* dst, const size_t sz_bytes, const size_t out_cap);
	const size_t readUTF8String(char32_t* dst, const size_t sz_bytes, const size_t out_cap);

	//If no BOM, then takes the streamer's endianness
	const size_t readUTF16String(UnicodeString& dst, const size_t sz_bytes, bool bom);
	const size_t readUTF16String(char16_t* dst, const size_t sz_bytes, const size_t out_cap, bool bom);
	const size_t readUTF16String(char32_t* dst, const size_t sz_bytes, const size_t out_cap, bool bom);

	virtual void close();

	virtual ~DataInputStreamer(){close();}

};

class WRCU_DLL_API InputException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	InputException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

class WRCU_DLL_API FileInputStreamer:public DataStreamerSource
{

private:
    const path sFilePath;

    ifstream* oOpenStream;
    size_t read = 0;
    size_t maxsz = 0; //Field to save file size so don't have to retrieve from OS when remainingBytes() is called.

public:

	FileInputStreamer(const path& path) :sFilePath(path), oOpenStream(nullptr) {}
    FileInputStreamer(const string& ascii_path):sFilePath(ascii_path),oOpenStream(nullptr){}
    FileInputStreamer(const char* ascii_path):sFilePath(ascii_path),oOpenStream(nullptr){}
    FileInputStreamer(const char16_t* utf16_path):sFilePath(utf16_path),oOpenStream(nullptr){}
    FileInputStreamer(const char32_t* utf32_path):sFilePath(utf32_path),oOpenStream(nullptr){}

    const path& getPath() const{return sFilePath;}

    const size_t fileSize() const;

    const bool isOpen() const override;
    const bool streamEnd() const override;
    const bool isGood() const;
    const bool isBad() const;
    const bool isFail() const;
    const size_t remaining() const override;
    const size_t bytesRead() const{return read;}

    void open() override;
    void open(const streampos startOffset);
    void jumpTo(const streampos offset);
    void skip(const streampos skip_amt);
    void close() override;
    const ifstream& getStreamView() const;

    const ubyte nextByte() override;

    virtual ~FileInputStreamer(){close();}

};

class WRCU_DLL_API DataOutputTarget
{

public:
	virtual const bool addByte(const ubyte b) = 0;
	virtual const bool addBytes(const ubyte* data, const size_t datlen) = 0;
	virtual const bool isOpen() const = 0;
	virtual void open() = 0;
    virtual void close() = 0;

	virtual ~DataOutputTarget(){}

};

class WRCU_DLL_API DataOutputStreamer
{

private:
	Endianness eEndian;
	DataOutputTarget& iTarget;
	bool flag_free_on_close = false;
	bool closed = false;

	int uconv_type = 0;
	UErrorCode uerr = U_ZERO_ERROR;
	UConverter* uconv = nullptr;

	void initconv_utf8();
	void initconv_utf32_sysordered();

public:
	DataOutputStreamer(DataOutputTarget& targ, const Endianness endian):eEndian(endian),iTarget(targ){}
	const Endianness getEndianness() const{return eEndian;}
	void setEndianness(const Endianness endian) { eEndian = endian; }

	void putByte(const ubyte value);
	void putUnsignedShort(const uint16_t value);
	void putShort(const int16_t value);
	void put24(const uint32_t value);
	void putUnsignedInt(const uint32_t value);
	void putInt(const int32_t value);
	void putUnsignedLong(const uint64_t value);
	void putLong(const int64_t value);

	const size_t putASCIIString(const string& src, const bool putLength, const bool padEven);
	const size_t putASCIIString(const UnicodeString& src, const bool putLength, const bool padEven);
	const size_t putASCIIString(const char* src, const size_t nChars, const bool putLength, const bool padEven);
	const size_t putASCIIString(const char16_t* src, const size_t nChars, const bool putLength, const bool padEven);
	const size_t putASCIIString(const char32_t* src, const size_t nChars, const bool putLength, const bool padEven);

	const size_t putUTF8String(const UnicodeString& src, const bool putLength, const bool padEven);
	const size_t putUTF8String(const char16_t* src, const size_t nChars, const bool putLength, const bool padEven);
	const size_t putUTF8String(const char32_t* src, const size_t nChars, const bool putLength, const bool padEven);

	const size_t putUTF16String(const UnicodeString& src, const bool putLength, const bool bom);
	const size_t putUTF16String(const char16_t* src, const size_t nChars, const bool putLength, const bool bom);
	const size_t putUTF16String(const char32_t* src, const size_t nChars, const bool putLength, const bool bom);

	void setFreeOnCloseFlag(bool b) { flag_free_on_close = b; }
	virtual void close();

	virtual ~DataOutputStreamer() { close(); }

};

class WRCU_DLL_API OutputException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	OutputException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

class WRCU_DLL_API FileOutputStreamer:public DataOutputTarget
{

private:
    const path sFilePath;

    ofstream* oOpenStream;

public:
	FileOutputStreamer(const path& path) :sFilePath(path), oOpenStream(nullptr) {}
    FileOutputStreamer(const string& ascii_path):sFilePath(ascii_path),oOpenStream(nullptr){}
    FileOutputStreamer(const char* ascii_path):sFilePath(ascii_path),oOpenStream(nullptr){}
    FileOutputStreamer(const char16_t* utf16_path):sFilePath(utf16_path),oOpenStream(nullptr){}
    FileOutputStreamer(const char32_t* utf32_path):sFilePath(utf32_path),oOpenStream(nullptr){}

    const path& getPath() const{return sFilePath;}

    const bool isOpen() const override;
    void open() override;
    void openForAppending();
    void close() override;
    const ofstream& getStreamView() const;

    const bool addByte(const ubyte value) override;
    const bool addBytes(const ubyte* data, const size_t datlen) override;

    virtual ~FileOutputStreamer(){close();}
};

class WRCU_DLL_API FileParsingException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	FileParsingException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};


}

#endif // FILESTREAMER_H_INCL

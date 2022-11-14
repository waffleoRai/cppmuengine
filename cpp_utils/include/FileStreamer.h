#ifndef FILESTREAMER_H_INCLUDED
#define FILESTREAMER_H_INCLUDED

#include "FileInput.h"
#include "FileOutput.h"

using std::ifstream;
using std::streampos;
using std::ofstream;
using std::cout;
using std::filesystem::path;
using icu::UnicodeString;

using waffleoRai_Utils::DataStreamerSource;
using waffleoRai_Utils::DataOutputTarget;

namespace waffleoRai_Utils {

enum class WRCU_DLL_API Endianness {little_endian,big_endian};

WRCU_DLL_API const bool WRCU_CDECL sys_endian_matches(Endianness e);

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

class WRCU_DLL_API FileParsingException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	FileParsingException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

/*----- Common Utility Functions -----*/

//Open piece of file as DataStreamerSource + release/close
//Open file/piece of file and wrap in DataInputStreamer + release/close
//Open DataOutputStreamer targeting a file + release/close
//Common IO/Exception handling function? (Printing messages or something?)
//Loading a file/file piece into memory

WRCU_DLL_API const bool open_file_as_input_source(FileInputStreamer& container, const streampos start_offset, const size_t len);
WRCU_DLL_API DataStreamerSource* open_file_as_input_source(const path& filepath, const streampos start_offset, const size_t len);
WRCU_DLL_API void close_file_as_input_source(DataStreamerSource* src);

WRCU_DLL_API DataInputStreamer* open_file_as_input_reader(const path& filepath, const streampos start_offset, const size_t len, const Endianness byte_order);
WRCU_DLL_API void close_file_as_input_reader(DataInputStreamer* src);

WRCU_DLL_API DataOutputStreamer* open_file_as_output_writer(const path& filepath, const Endianness byte_order);
WRCU_DLL_API void close_file_as_output_writer(DataOutputStreamer* trg);

WRCU_DLL_API const size_t load_file_to_memory(const path& filepath, const streampos start_offset, const size_t len, void* dst);
WRCU_DLL_API const size_t load_file_to_memory(const path& filepath, void* dst);
WRCU_DLL_API const size_t write_file_from_memory(const path& filepath, const void* src, const size_t len);

};

#endif // FILESTREAMER_H_INCL

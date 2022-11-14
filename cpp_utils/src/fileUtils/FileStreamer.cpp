#include "FileStreamer.h"

using std::filesystem::filesystem_error;

namespace waffleoRai_Utils
{

const bool sys_endian_matches(Endianness e) {
		if (e == Endianness::little_endian) return !wrcu_sys_big_endian();
		else return wrcu_sys_big_endian();
}

/*----- Common Utility -----*/

const bool open_file_as_input_source(FileInputStreamer& container, const streampos start_offset, const size_t len) {
	if (container.isOpen()) return false;
	container.open(start_offset);
	container.setEndBoundary(start_offset + static_cast<streampos>(len));
	return true;
}

DataStreamerSource* open_file_as_input_source(const path& filepath, const streampos start_offset, const size_t len) {
	FileInputStreamer* fis = new FileInputStreamer(filepath);
	if (!fis) return NULL;
	fis->open(start_offset);
	fis->setEndBoundary(start_offset + static_cast<streampos>(len));
	return fis;
}

void close_file_as_input_source(DataStreamerSource* src) {
	if (!src) return;
	src->close();
	delete src;
}

DataInputStreamer* open_file_as_input_reader(const path& filepath, const streampos start_offset, const size_t len, const Endianness byte_order) {
	DataInputStreamer* dis = NULL;
	DataStreamerSource* src = open_file_as_input_source(filepath, start_offset, len);
	if (!src) return NULL;
	dis = new DataInputStreamer(*src, byte_order);
	dis->setFreeOnCloseFlag(true);
	return dis;
}

void close_file_as_input_reader(DataInputStreamer* src) {
	if (!src) return;
	src->close();
	delete src;
}

DataOutputStreamer* open_file_as_output_writer(const path& filepath, const Endianness byte_order) {
	DataOutputStreamer* dos = NULL;
	FileOutputStreamer* output = new FileOutputStreamer(filepath);
	if (!output) return NULL;
	dos = new DataOutputStreamer(*output, byte_order);
	dos->setFreeOnCloseFlag(true);
	return dos;
}

void close_file_as_output_writer(DataOutputStreamer* trg) {
	if (!trg) return;
	trg->close();
	delete trg;
}

const size_t load_file_to_memory(const path& filepath, const streampos start_offset, const size_t len, void* dst) {
	if (!dst) return 0;
	ifstream input(filepath);
	streampos start, current;
	input.seekg(start_offset, std::ios_base::beg);
	start = input.tellg();
	input.read((char*)dst, len);
	current = input.tellg();
	input.close();
	return static_cast<size_t>(current - start);
}

const size_t load_file_to_memory(const path& filepath, void* dst) {
	if (!dst) return 0;
	ifstream input(filepath);
	size_t read = 0;
	ubyte* dest = (ubyte*)dst;
	int b = EOF;
	while ((b = input.get()) != EOF) {
		*dest++ = b;
		read++;
	}
	input.close();
	return read;
}

const size_t write_file_from_memory(const path& filepath, const void* src, const size_t len) {
	if (!src | len < 1) return 0;
	streampos current;
	ofstream output(filepath);
	output.write((const char*)src, len);
	current = output.tellp();
	output.close();
	return static_cast<size_t>(current);
}

/*----- DataInputStreamer -----*/

const uint16_t DataInputStreamer::nextUnsignedShort(){
	const int bCount = 2;
	uint16_t out = 0;
	ubyte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.nextByte();
	if (eEndian == Endianness::big_endian){
		//Start at beginning
		for(int i = 0; i < bCount; i++){
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == Endianness::little_endian){
		//Start at end
		for(int i = bCount-1; i >= 0; i--){
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const int16_t DataInputStreamer::nextShort(){
	return (x16)nextUnsignedShort();
}

const uint32_t DataInputStreamer::nextUnsignedInt(){
	const int bCount = 4;
	uint32_t out = 0;
	ubyte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.nextByte();
	if (eEndian == Endianness::big_endian){
		//Start at beginning
		for(int i = 0; i < bCount; i++){
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == Endianness::little_endian){
		//Start at end
		for(int i = bCount-1; i >= 0; i--){
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const int32_t DataInputStreamer::nextInt(){
	return (x32)nextUnsignedInt();
}

const uint32_t DataInputStreamer::nextUnsigned24(){
	const int bCount = 3;
	uint32_t out = 0;
	ubyte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.nextByte();
	if (eEndian == Endianness::big_endian){
		//Start at beginning
		for(int i = 0; i < bCount; i++){
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == Endianness::little_endian){
		//Start at end
		for(int i = bCount-1; i >= 0; i--){
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const int32_t DataInputStreamer::next24(){
	uint32_t myint = nextUnsigned24();
	if ((myint & 0x800000) == 0) return (int32_t)myint;
	return (int32_t)(myint | 0xFFFF0000);
}

const uint64_t DataInputStreamer::nextUnsignedLong(){
	const int bCount = 8;
	uint64_t out = 0;
	ubyte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.nextByte();
	if (eEndian == Endianness::big_endian){
		//Start at beginning
		for(int i = 0; i < bCount; i++){
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == Endianness::little_endian){
		//Start at end
		for(int i = bCount-1; i >= 0; i--){
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const int64_t DataInputStreamer::nextLong(){
	return (x64)nextUnsignedLong();
}

const size_t DataInputStreamer::skip(streampos amt){
	//NEEDS to check if at end!!!
	size_t ct = 0;
	int i = 0;
    for(i = 0; i < amt; i++){
		if (iSource.streamEnd()) return ct;
        iSource.nextByte();
        ct++;
    }
    return ct;
}

void DataInputStreamer::close(){
    if(closed) return;
    //iSource.close(); //This is commented out because it should be handled by the free on close flag. Otherwise this causes issues if the source is deleted first.
    if(flag_free_on_close) delete &iSource;
	if (uconv) ucnv_close(uconv);
    closed = true;
}

/*----- DataOutputStreamer -----*/

void DataOutputStreamer::putByte(const ubyte value){
	iTarget.addByte(value);
}

void DataOutputStreamer::putUnsignedShort(const uint16_t value){
	const int bcount = 2;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian){
	case Endianness::big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	case Endianness::little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	}
}

void DataOutputStreamer::putShort(const int16_t value){
	putUnsignedShort((u16)value);
}

void DataOutputStreamer::put24(const uint32_t value)
{
	const int bcount = 3;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian){
	case Endianness::big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	case Endianness::little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	}
}

void DataOutputStreamer::putUnsignedInt(const uint32_t value)
{
	const int bcount = 4;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian){
	case Endianness::big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	case Endianness::little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	}
}

void DataOutputStreamer::putInt(const int32_t value){
	putUnsignedInt((u32)value);
}

void DataOutputStreamer::putUnsignedLong(const uint64_t value){
	const int bcount = 8;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian){
	case Endianness::big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	case Endianness::little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((ubyte)((value >> sh) & 0xFF));
		break;
	}

}

void DataOutputStreamer::putLong(const int64_t value){
	putUnsignedLong((u64)value);
}

void DataOutputStreamer::close() {
	if (closed) return;
	iTarget.close();
	if (flag_free_on_close) delete &iTarget;
	if (uconv) ucnv_close(uconv);
	closed = true;
}

/*----- String Readers -----*/

void DataInputStreamer::initconv_utf8() {
	if (uconv) ucnv_close(uconv);
	uconv = ucnv_open("UTF8", &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::initconv_utf8", "UTF8 converter init failed", uerr);
	}
	uconv_type = 8;
}

void DataInputStreamer::initconv_utf16_bom() {
	if (uconv) ucnv_close(uconv);
	uconv = ucnv_open("UTF16", &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::initconv_utf16_bom", "UTF16(BOM) converter init failed", uerr);
	}
	uconv_type = 16;
}

void DataInputStreamer::initconv_utf16_ordered() {
	if (uconv) ucnv_close(uconv);
	if(eEndian == Endianness::big_endian) uconv = ucnv_open("UTF16BE", &uerr);
	else uconv = ucnv_open("UTF16LE", &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::initconv_utf16_ordered", "UTF16 converter init failed", uerr);
	}
	uconv_type = 15;
}

void DataInputStreamer::initconv_utf32_sysordered() {
	if (uconv) ucnv_close(uconv);
	if (wrcu_sys_big_endian()) uconv = ucnv_open("UTF32BE", &uerr);
	else uconv = ucnv_open("UTF32LE", &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::initconv_utf32_sysordered", "UTF32 converter init failed", uerr);
	}
	uconv_type = 32;
}

const size_t DataInputStreamer::readASCIIString(string& dst, const size_t sz_bytes) {
	size_t i;
	for (i = 0; i < sz_bytes; i++) {
		if (streamEnd()) return i;
		dst.push_back(static_cast<char>(nextByte()));
	}
	return sz_bytes;
}

const size_t DataInputStreamer::readASCIIString(UnicodeString& dst, const size_t sz_bytes) {
	return readUTF8String(dst, sz_bytes);
}

const size_t DataInputStreamer::readASCIIString(char16_t* dst, const size_t sz_bytes) {
	size_t i;
	char16_t* pos = dst;
	for (i = 0; i < sz_bytes; i++) {
		if (streamEnd()) return i;
		*(pos++) = static_cast<char16_t>(nextByte());
	}
	return sz_bytes;
}

const size_t DataInputStreamer::readASCIIString(char32_t* dst, const size_t sz_bytes) {
	size_t i;
	char32_t* pos = dst;
	for (i = 0; i < sz_bytes; i++) {
		if (streamEnd()) return i;
		*(pos++) = static_cast<char32_t>(nextByte());
	}
	return sz_bytes;
}

const size_t DataInputStreamer::readUTF8String(UnicodeString& dst, const size_t sz_bytes) {
	//I guess I'll just try writing it to the UnicodeString's buffer directly?
	int32_t amt = static_cast<int32_t>(sz_bytes+1) << 2; //Overkill, but eh
	char16_t* dst_buff = dst.getBuffer(amt);
	if (uconv_type != 8) initconv_utf8();
	char* bbuff = (char*)malloc(sz_bytes+1); //Don't forget null char
	*(bbuff + sz_bytes) = '\0';
	const size_t cpy = nextBytes((ubyte*)bbuff, sz_bytes);

	int32_t outcnt = ucnv_toUChars(uconv, reinterpret_cast<UChar*>(dst_buff), amt>>1, bbuff, -1, &uerr);
	free(bbuff);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		dst.releaseBuffer(0);
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::readUTF8String", "Unicode conversion failed", uerr);
	}
	dst.releaseBuffer(outcnt);
	return outcnt;
}

const size_t DataInputStreamer::readUTF8String(char16_t* dst, const size_t sz_bytes, const size_t out_cap) {
	if (!dst || sz_bytes <= 0 || out_cap <= 0) return 0;

	if (uconv_type != 8) initconv_utf8();
	char* bbuff = (char*)malloc(sz_bytes + 1);
	*(bbuff + sz_bytes) = '\0';
	const size_t cpy = nextBytes((ubyte*)bbuff, sz_bytes);

	int32_t outcnt = ucnv_toUChars(uconv, dst, out_cap >> 1, bbuff, -1, &uerr);
	free(bbuff);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::readUTF8String", "Unicode conversion failed", uerr);
	}
	return outcnt;
}

const size_t DataInputStreamer::readUTF8String(char32_t* dst, const size_t sz_bytes, const size_t out_cap) {
	if (!dst || sz_bytes <= 0 || out_cap <= 0) return 0;
	size_t tcap = (out_cap + 2);
	char16_t* tbuff = (char16_t*)malloc(tcap);
	size_t outcnt = readUTF8String(tbuff, sz_bytes, tcap);

	//Now to UTF32...
	initconv_utf32_sysordered();
	int32_t out32 = ucnv_fromUChars(uconv, reinterpret_cast<char*>(dst), out_cap >> 2, reinterpret_cast<UChar*>(tbuff), static_cast<int32_t>(outcnt), &uerr);
	free(tbuff);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::readUTF8String", "Unicode conversion failed", uerr);
	}

	return out32;
}

//If no BOM, then takes the streamer's endianness
//I reckon if it matches the system endianness, can just cheese it and copy directly.......
const size_t DataInputStreamer::readUTF16String(UnicodeString& dst, const size_t sz_bytes, bool bom) {
	int32_t amt = static_cast<int32_t>(sz_bytes +2);
	char16_t* dst_buff = dst.getBuffer(amt>>1);
	size_t outcnt = readUTF16String(dst_buff, sz_bytes, amt, bom);
	dst.releaseBuffer(outcnt);
	return outcnt;
}

const size_t DataInputStreamer::readUTF16String(char16_t* dst, const size_t sz_bytes, const size_t out_cap, bool bom) {
	//Like... can just copy it, yeah?
	bool ematch = false;
	size_t rembytes = sz_bytes;
	if (bom) {
		//Read the BOM to check
		uint16_t cp_bom = 0;
		ubyte* ptr = (ubyte*)&cp_bom;
		*ptr = nextByte();
		*(ptr + 1) = nextByte();
		ematch = (cp_bom == 0xFEFF);
		rembytes -= 2;
	}
	else {
		//Just check against the streamer's endianness
		ematch = sys_endian_matches(eEndian);
	}

	size_t outchar = 0;
	size_t i;
	ubyte* ptr = reinterpret_cast<ubyte*>(dst);
	ubyte* lim = ptr + out_cap;
	if (ematch) {
		//Big copy!
		for (i = 0; i < rembytes; i += 2) {
			if (ptr >= lim) break;
			*(ptr++) = nextByte();
			*(ptr++) = nextByte();
			outchar++;
		}
	}
	else {
		for (i = 0; i < rembytes; i += 2) {
			if (ptr >= lim) break;
			*(ptr+1) = nextByte();
			*ptr = nextByte();
			ptr += 2;
			outchar++;
		}
	}
	if (ptr < lim) {
		*(ptr++) = 0;
		*(ptr++) = 0;
	}

	return outchar;
}

const size_t DataInputStreamer::readUTF16String(char32_t* dst, const size_t sz_bytes, const size_t out_cap, bool bom) {
	//First read to char16_t...
	if (!dst || sz_bytes <= 0 || out_cap <= 0) return 0;
	size_t tcap = (out_cap + 2); //Temp size only needs to be about half UTF32 buffer size, but some cps are 4-bytes, so leave the space just in case.
	char16_t* tbuff = (char16_t*)malloc(tcap);
	size_t out16 = readUTF16String(tbuff, sz_bytes, tcap, bom); //out16 should now be the number of UTF16 chars, barring null at end

	//Then to char32_t...
	if (uconv_type != 32) initconv_utf32_sysordered();
	//**I don't know if dst capacity should be in bytes, UChars, or target encoding chars. I have scaled it to the last one for now?
	int32_t out32 = ucnv_fromUChars(uconv, reinterpret_cast<char*>(dst), out_cap >> 2, reinterpret_cast<UChar*>(tbuff), static_cast<int32_t>(out16), &uerr);
	free(tbuff);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataInputStreamer::readUTF16String", "Unicode conversion failed", uerr);
	}

	return out32;
}

/*----- String Writers -----*/

void DataOutputStreamer::initconv_utf8() {
	if (uconv) ucnv_close(uconv);
	uconv = ucnv_open("UTF8", &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataOutputStreamer::initconv_utf8", "UTF8 converter init failed", uerr);
	}
	uconv_type = 8;
}

void DataOutputStreamer::initconv_utf32_sysordered() {
	if (uconv) ucnv_close(uconv);
	if (wrcu_sys_big_endian()) uconv = ucnv_open("UTF32BE", &uerr);
	else uconv = ucnv_open("UTF32LE", &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		throw UnicodeConversionException("waffleoRai_Utils::DataOutputStreamer::initconv_utf32_sysordered", "UTF32 converter init failed", uerr);
	}
	uconv_type = 32;
}

const size_t DataOutputStreamer::putASCIIString(const string& src, const bool putLength, const bool padEven) {
	size_t strlen = src.length();
	size_t ct = 0;
	if (putLength) { putUnsignedShort(static_cast<uint16_t>(strlen)); ct += 2; }

	size_t i;
	for (i = 0; i < strlen; i++) {
		putByte(static_cast<ubyte>(src[i])); ct++;
	}

	if (padEven && (strlen % 2 != 0)) { putByte(0); ct++; }

	return ct;
}

const size_t DataOutputStreamer::putASCIIString(const UnicodeString& src, const bool putLength, const bool padEven) {
	int32_t strlen = src.length();
	size_t ct = 0;
	if (putLength) { putUnsignedShort(static_cast<uint16_t>(strlen)); ct += 2; }

	int32_t i;
	for (i = 0; i < strlen; i++) {
		putByte(static_cast<ubyte>(src[i])); ct++;
	}
	if (padEven && (strlen % 2 != 0)) { putByte(0); ct++; }

	return ct;
}

const size_t DataOutputStreamer::putASCIIString(const char* src, const size_t nChars, const bool putLength, const bool padEven) {
	if (!src || nChars <= 0) return 0;
	size_t ct = 0;
	if (putLength) { putUnsignedShort(static_cast<uint16_t>(nChars)); ct += 2; }

	size_t i;
	for (i = 0; i < nChars; i++) {
		putByte(static_cast<ubyte>(*(src+i))); ct++;
	}

	if (padEven && (nChars % 2 != 0)) { putByte(0); ct++; }

	return ct;
}

const size_t DataOutputStreamer::putASCIIString(const char16_t* src, const size_t nChars, const bool putLength, const bool padEven) {
	if (!src || nChars <= 0) return 0;
	size_t ct = 0;
	if (putLength) { putUnsignedShort(static_cast<uint16_t>(nChars)); ct += 2; }

	size_t i;
	for (i = 0; i < nChars; i++) {
		putByte(static_cast<ubyte>(*(src + i))); ct++;
	}

	if (padEven && (nChars % 2 != 0)) { putByte(0); ct++; }

	return ct;
}

const size_t DataOutputStreamer::putASCIIString(const char32_t* src, const size_t nChars, const bool putLength, const bool padEven) {
	if (!src || nChars <= 0) return 0;
	size_t ct = 0;
	if (putLength) { putUnsignedShort(static_cast<uint16_t>(nChars)); ct += 2; }

	size_t i;
	for (i = 0; i < nChars; i++) {
		putByte(static_cast<ubyte>(*(src + i))); ct++;
	}

	if (padEven && (nChars % 2 != 0)) { putByte(0); ct++; }

	return ct;
}

const size_t DataOutputStreamer::putUTF8String(const UnicodeString& src, const bool putLength, const bool padEven) {
	return putUTF8String(src.getBuffer(), static_cast<size_t>(src.length()), putLength, padEven);
}

const size_t DataOutputStreamer::putUTF8String(const char16_t* src, const size_t nChars, const bool putLength, const bool padEven) {
	if (!src || nChars <= 0) return 0;
	size_t ct = 0;
	//(Length that is put is output length in bytes)
	//So encode first

	int32_t outcap = nChars << 2;
	char* outbuff = (char*)malloc(outcap); //Giving it 4x input char count. Probably don't need that much
	if (uconv_type != 8) initconv_utf8();
	int32_t outlen = ucnv_fromUChars(uconv, outbuff, outcap, src, nChars, &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		free(outbuff);
		throw UnicodeConversionException("waffleoRai_Utils::DataOutputStreamer::putUTF8String", "Unicode conversion failed", uerr);
	}

	ubyte* bptr = reinterpret_cast<ubyte*>(outbuff);
	if (putLength) { putUnsignedShort(static_cast<uint16_t>(outlen)); ct += 2; }
	int32_t i;
	for (i = 0; i < outlen; i++) {
		putByte(*bptr++); ct++;
	}
	if (padEven && (outlen % 2 != 0)) { putByte(0); ct++; }
	free(outbuff);

	return ct;
}

const size_t DataOutputStreamer::putUTF8String(const char32_t* src, const size_t nChars, const bool putLength, const bool padEven) {
	if (!src || nChars <= 0) return 0;

	//Convert to unicode string, then pass to char16 overload
	char16_t* ubuff = (char16_t*)malloc(nChars);
	if (uconv_type != 32) initconv_utf32_sysordered();
	int32_t words = ucnv_toUChars(uconv, ubuff, nChars, reinterpret_cast<const char*>(src), nChars << 2, &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		free(ubuff);
		throw UnicodeConversionException("waffleoRai_Utils::DataOutputStreamer::putUTF8String", "Unicode conversion failed", uerr);
	}

	size_t result = 0;
	try {
		result = putUTF8String(ubuff, static_cast<size_t>(words), putLength, padEven);
	}
	catch (UnicodeConversionException& ex) {
		free(ubuff);
		throw ex;
	}

	free(ubuff);
	return result;
}

const size_t DataOutputStreamer::putUTF16String(const UnicodeString& src, const bool putLength, const bool bom) {
	return putUTF16String(src.getBuffer(), static_cast<size_t>(src.length()), putLength, bom);
}

const size_t DataOutputStreamer::putUTF16String(const char16_t* src, const size_t nChars, const bool putLength, const bool bom) {
	//Depending on byte order, may just be able to copy.
	if (!src || nChars <= 0) return 0;
	size_t ct = 0;
	if (putLength) {
		int16_t outsize = static_cast<int16_t>(nChars << 1);
		if (bom) outsize += 2;
		putUnsignedShort(outsize);
		ct += 2;
	}

	if (bom) putUnsignedShort(0xFEFF);

	size_t i;
	size_t bcount = nChars << 1;
	if (sys_endian_matches(eEndian)) {
		iTarget.addBytes(reinterpret_cast<const ubyte*>(src), bcount);
		ct += bcount;
	}
	else {
		for (i = 0; i < nChars; i++) {
			putByte(*(src+1));
			putByte(*src);
			ct += 2; src += 2;
		}
	}

	return ct;
}

const size_t DataOutputStreamer::putUTF16String(const char32_t* src, const size_t nChars, const bool putLength, const bool bom) {
	if (!src || nChars <= 0) return 0;

	//Convert to unicode string, then pass to char16 overload
	char16_t* ubuff = (char16_t*)malloc(nChars);
	if (uconv_type != 32) initconv_utf32_sysordered();
	int32_t words = ucnv_toUChars(uconv, ubuff, nChars, reinterpret_cast<const char*>(src), nChars << 2, &uerr);
	if (uerr != U_ZERO_ERROR) {
		uconv_type = -1;
		free(ubuff);
		throw UnicodeConversionException("waffleoRai_Utils::DataOutputStreamer::putUTF8String", "Unicode conversion failed", uerr);
	}

	size_t result = 0;
	try {
		result = putUTF16String(ubuff, static_cast<size_t>(words), putLength, bom);
	}
	catch (UnicodeConversionException& ex) {
		free(ubuff);
		throw ex;
	}

	free(ubuff);
	return result;
}

}

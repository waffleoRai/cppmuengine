#include "FileStreamer.h"

using namespace std;

namespace waffleoRai_Utils
{

const bool sys_endian_matches(Endianness e) {
		if (e == Endianness::little_endian) return !wrcu_sys_big_endian();
		else return wrcu_sys_big_endian();
}

const size_t  DataStreamerSource::nextBytes(ubyte* dst, const size_t len) {
	if (!dst || len <= 0) return 0;
	size_t i;
	ubyte* pos = dst;
	for (i = 0; i < len; i++) { 
		if(streamEnd()) return static_cast<size_t>(pos - dst);
		*(pos++) = nextByte(); 
	}

	return static_cast<size_t>(pos-dst);
}

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
	size_t ct = 0;
	int i = 0;
    for(i = 0; i < amt; i++){
        iSource.nextByte();
        ct++;
    }
    return ct;
}

void DataInputStreamer::close(){
    if(closed) return;
    iSource.close();
    if(flag_free_on_close) delete &iSource;
	if (uconv) ucnv_close(uconv);
    closed = true;
}

const size_t FileInputStreamer::fileSize() const{
	//https://stackoverflow.com/questions/5840148/how-can-i-get-a-files-size-in-c
	/*struct stat stat_buf;
	int rc = stat(sFilePath, &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;*/

	try {
		return static_cast<size_t>(filesystem::file_size(sFilePath));
	}
	catch (filesystem_error& x) { return -1LL; }
}

const bool FileInputStreamer::isOpen() const{
    return (oOpenStream != NULL && oOpenStream->is_open());
}

const bool FileInputStreamer::streamEnd() const{
    if(oOpenStream == NULL) return true;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->eof();
}

const bool FileInputStreamer::isGood() const{
    if(oOpenStream == NULL) return false;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->good();
}

const bool FileInputStreamer::isBad() const{
    if(oOpenStream == NULL) return false;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->bad();
}

const bool FileInputStreamer::isFail() const{
    if(oOpenStream == NULL) return false;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->fail();
}

const size_t FileInputStreamer::remaining() const{
	if(!isOpen()) return 0;
	if(maxsz < 0ULL) return 0;
	size_t diff = maxsz - static_cast<size_t>(oOpenStream->tellg());
	if(diff < 0ULL) return 0;
	return diff;
}

const ifstream& FileInputStreamer::getStreamView() const{
	if(oOpenStream == NULL) throw InputException("waffleoRai_Utils::FileInputStreamer::getStreamView","Stream is not open!");
	return *oOpenStream;
}

void FileInputStreamer::open(){
	//Maybe set exception throwing instead of doing active check?
	if(isOpen()) return;
	read = 0;
	maxsz = fileSize();
	oOpenStream = new ifstream(sFilePath, ifstream::in|ifstream::binary);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Failed to open stream!");
}

void FileInputStreamer::open(const streampos startOffset){
	if(isOpen()) return;
	read = 0;
	maxsz = fileSize();
	if(startOffset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Open offset after end of file!");
	oOpenStream = new ifstream(sFilePath, ifstream::in|ifstream::binary);
	oOpenStream->seekg(startOffset);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Failed to open stream!");
}

void FileInputStreamer::jumpTo(const streampos offset){
	if(!isOpen()) return;
	if(offset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::jumpTo","Target offset after end of file!");
	oOpenStream->seekg(offset);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::jumpTo","Stream seekg failed!");
}

void FileInputStreamer::skip(const streampos skip_amt){
	streampos pos = oOpenStream->tellg();
	jumpTo(pos + skip_amt);
}

void FileInputStreamer::close(){
	if(oOpenStream){
		oOpenStream->close();
		if(oOpenStream->is_open()) throw InputException("waffleoRai_Utils::FileInputStreamer::close","Failed to close stream!");
		delete oOpenStream;
		oOpenStream = nullptr;
	}
}

const ubyte FileInputStreamer::nextByte(){
	if(!isOpen()) throw InputException("waffleoRai_Utils::FileInputStreamer::nextByte","Failed to retrieve next byte - stream is not open!");
	int b = oOpenStream->get();
	//if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::nextByte","Failed to retrieve next byte!");
	read++;
	return (ubyte)b;
}

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

const bool FileOutputStreamer::isOpen() const{
	if(oOpenStream == NULL) return false;
	return oOpenStream->is_open();
}

void FileOutputStreamer::open(){
	close();
	oOpenStream = new ofstream(sFilePath.c_str(), (ofstream::out | ofstream::binary));
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileOutputStreamer::open","Failed to open stream!");
}

void FileOutputStreamer::openForAppending(){
	close();
	oOpenStream = new ofstream(sFilePath.c_str(), (ofstream::out | ofstream::binary | ofstream::app));
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileOutputStreamer::openForAppending","Failed to open stream!");
}

void FileOutputStreamer::close(){
	if(oOpenStream != NULL){
		oOpenStream->close();
		if(oOpenStream->is_open()) throw InputException("waffleoRai_Utils::FileOutputStreamer::close","Failed to close stream!");
		delete oOpenStream;
		oOpenStream = NULL;
	}
}

const ofstream& FileOutputStreamer::getStreamView() const{
	if(oOpenStream == NULL) throw OutputException("waffleoRai_Utils::FileOutputStreamer::getStreamView","Stream is not open!");
	return *oOpenStream;
}

const bool FileOutputStreamer::addByte(const ubyte value){
	//if(!isOpen()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::addByte","Failed to add byte - stream is not open!");
	if(!isOpen()) return false;
	oOpenStream->put(value);
	//if(oOpenStream->fail()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::addByte","Failed to add byte!");
	return true;
}

const bool FileOutputStreamer::addBytes(const ubyte* data, const size_t datlen){
	if(!isOpen()) return false;
	oOpenStream->write(reinterpret_cast<const char*>(data), datlen);
	return true;
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

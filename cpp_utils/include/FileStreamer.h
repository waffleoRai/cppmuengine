#ifndef FILESTREAMER_H_INCLUDED
#define FILESTREAMER_H_INCLUDED

#include "quickDefs.h"
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <exception>

using namespace std;

namespace waffleoRai_Utils
{

enum Endianness {little_endian,big_endian};

class DataStreamerSource
{

public:
	virtual const byte nextByte() = 0;
	virtual const u64 remaining() const = 0;
	virtual const bool streamEnd() const = 0;

	virtual void open() = 0;
	virtual void close() = 0;

	virtual ~DataStreamerSource() = 0;
};

class DataInputStreamer
{

private:
	Endianness eEndian;
	DataStreamerSource& iSource;
	bool flag_free_on_close = false;
	bool closed = false;

public:
	DataInputStreamer(DataStreamerSource& src, const Endianness endian):eEndian(endian),iSource(src){}

	DataStreamerSource& getSource() {return iSource;}
	const Endianness getEndianness() const{return eEndian;}
	void setEndianness(const Endianness endian){eEndian = endian;}
	virtual const u64 remaining() const{return iSource.remaining();}
	virtual const bool streamEnd() const{return iSource.streamEnd();}

	const u64 skip(u64 amt);

	const byte nextByte(){return iSource.nextByte();}
	const u16 nextUnsignedShort();
	const x16 nextShort();
	const u32 nextUnsigned24();
	const x32 next24();
	const u32 nextUnsignedInt();
	const x32 nextInt();
	const u64 nextUnsignedLong();
	const x64 nextLong();

	void setFreeOnCloseFlag(bool b){flag_free_on_close = b;}

	virtual void close();

	virtual ~DataInputStreamer(){close();}

};

class InputException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	InputException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

class FileInputStreamer:public DataStreamerSource
{

private:
    const string sFilePath;
    //const Endianness eEndian;

    ifstream* oOpenStream;
    u64 read = 0;
    u64 maxsz = 0; //Field to save file size so don't have to retrieve from OS when remainingBytes() is called.

public:

    FileInputStreamer(const string& path):sFilePath(path){oOpenStream = NULL;}

    const string& getPath() const{return sFilePath;}

    const x64 fileSize() const;

    const bool isOpen() const;
    const bool streamEnd() const override;
    const bool isGood() const;
    const bool isBad() const;
    const bool isFail() const;
    const u64 remaining() const override;
    const u64 bytesRead() const{return read;}

    void open() override;
    void open(const u32 startOffset);
    void open(const u64 startOffset);
    void jumpTo(const u64 offset);
    void skip(const u64 skip_amt);
    void close() override;
    const ifstream& getStreamView() const;

    const byte nextByte() override;

    virtual ~FileInputStreamer(){close();}

};

class DataOutputTarget
{

public:
	virtual const bool addByte(byte b) = 0;
	virtual const bool addBytes(const char* data, long datlen) = 0;
	//virtual const bool appendable() = 0;
	virtual ~DataOutputTarget() = 0;

};

class DataOutputStreamer
{

private:
	const Endianness eEndian;
	DataOutputTarget& iTarget;

public:
	DataOutputStreamer(const Endianness endian, DataOutputTarget& targ):eEndian(endian),iTarget(targ){}
	const Endianness getEndianness() const{return eEndian;}

	void putByte(byte value);
	void putUnsignedShort(u16 value);
	void putShort(x16 value);
	void put24(u32 value);
	void putUnsignedInt(u32 value);
	void putInt(x32 value);
	void putUnsignedLong(u64 value);
	void putLong(x64 value);

};

class OutputException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	OutputException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

class FileOutputStreamer:public DataOutputStreamer, public DataOutputTarget
{

private:
    const string sFilePath;

    ofstream* oOpenStream;

public:
    FileOutputStreamer(const string& path, Endianness e):DataOutputStreamer(e, *this),sFilePath(path){oOpenStream = NULL;}

    const string& getPath() const{return sFilePath;}

    const bool isOpen() const;
    void open();
    void openForAppending();
    void close();
    const ofstream& getStreamView() const;

    const bool addByte(byte value) override;
    const bool addBytes(const char* data, long datlen) override;

    virtual ~FileOutputStreamer(){close();}
};

class FileParsingException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	FileParsingException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

class NullPointerException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
	NullPointerException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

}

#endif // FILESTREAMER_H_INCLUDED

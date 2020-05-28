#include "FileStreamer.h"

using namespace std;

namespace waffleoRai_Utils
{

const u16 DataInputStreamer::nextUnsignedShort()
{
	const int bCount = 2;
	u16 out = 0;
	byte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.getNextByte();
	if (eEndian == big_endian)
	{
		//Start at beginning
		for(int i = 0; i < bCount; i++)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == little_endian)
	{
		//Start at end
		for(int i = bCount-1; i >= 0; i--)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const x16 DataInputStreamer::nextShort()
{
	return (x16)nextUnsignedShort();
}

const u32 DataInputStreamer::nextUnsignedInt()
{
	const int bCount = 4;
	u32 out = 0;
	byte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.getNextByte();
	if (eEndian == big_endian)
	{
		//Start at beginning
		for(int i = 0; i < bCount; i++)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == little_endian)
	{
		//Start at end
		for(int i = bCount-1; i >= 0; i--)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const x32 DataInputStreamer::nextInt()
{
	return (x32)nextUnsignedInt();
}

const u32 DataInputStreamer::nextUnsigned24()
{
	const int bCount = 3;
	u32 out = 0;
	byte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.getNextByte();
	if (eEndian == big_endian)
	{
		//Start at beginning
		for(int i = 0; i < bCount; i++)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == little_endian)
	{
		//Start at end
		for(int i = bCount-1; i >= 0; i--)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const x32 DataInputStreamer::next24()
{
	u32 myint = nextUnsigned24();
	if ((myint & 0x800000) == 0) return (x32)myint;
	return (x32)(myint | 0xFFFF0000);
}

const u64 DataInputStreamer::nextUnsignedLong()
{
	const int bCount = 8;
	u64 out = 0;
	byte barr[bCount];

	for(int i = 0; i < bCount; i++) barr[i] = iSource.getNextByte();
	if (eEndian == big_endian)
	{
		//Start at beginning
		for(int i = 0; i < bCount; i++)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	else if (eEndian == little_endian)
	{
		//Start at end
		for(int i = bCount-1; i >= 0; i--)
		{
			out = out<<8;
			out |= barr[i];
		}
	}
	return out;
}

const x64 DataInputStreamer::nextLong()
{
	return (x64)nextUnsignedLong();
}

const x64 FileInputStreamer::fileSize() const
{
	//https://stackoverflow.com/questions/5840148/how-can-i-get-a-files-size-in-c
	struct stat stat_buf;
	int rc = stat(sFilePath.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

const bool FileInputStreamer::isOpen() const
{
    return (oOpenStream != NULL && oOpenStream->is_open());
}

const bool FileInputStreamer::streamEnd() const
{
    if(oOpenStream == NULL) return true;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->eof();
}

const bool FileInputStreamer::isGood() const
{
    if(oOpenStream == NULL) return false;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->good();
}

const bool FileInputStreamer::isBad() const
{
    if(oOpenStream == NULL) return false;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->bad();
}

const bool FileInputStreamer::isFail() const
{
    if(oOpenStream == NULL) return false;
    if(!oOpenStream->is_open()) return true;
    return oOpenStream->fail();
}

const u64 FileInputStreamer::remainingBytes() const
{
	if(!isOpen()) return 0;
	//x64 fsz = fileSize();
	if(maxsz < 0L) return 0;
	u64 diff = maxsz - oOpenStream->tellg();
	if(diff < 0L) return 0;
	return diff;
}

const ifstream& FileInputStreamer::getStreamView() const
{
	if(oOpenStream == NULL) throw InputException("waffleoRai_Utils::FileInputStreamer::getStreamView","Stream is not open!");
	return *oOpenStream;
}

void FileInputStreamer::open()
{
	//Maybe set exception throwing instead of doing active check?
	if(isOpen()) return;
	read = 0;
	maxsz = fileSize();
	oOpenStream = new ifstream(sFilePath.c_str(), ifstream::in|ifstream::binary);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Failed to open stream!");
}

void FileInputStreamer::open(const u32 startOffset)
{
	if(isOpen()) return;
	read = 0;
	maxsz = fileSize();
	if(startOffset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Open offset after end of file!");
	oOpenStream = new ifstream(sFilePath.c_str(), ifstream::in|ifstream::binary);
	oOpenStream->seekg(startOffset);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Failed to open stream!");
}

void FileInputStreamer::open(const u64 startOffset)
{
	if(isOpen()) return;
	read = 0;
	maxsz = fileSize();
	if(startOffset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Open offset after end of file!");
	oOpenStream = new ifstream(sFilePath.c_str(), ifstream::in|ifstream::binary);
	oOpenStream->seekg(startOffset);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Failed to open stream!");
}

void FileInputStreamer::jumpTo(const u64 offset)
{
	if(!isOpen()) return;
	if(offset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open","Target offset after end of file!");
	oOpenStream->seekg(offset);
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::jumpTo","Stream seekg failed!");
}

void FileInputStreamer::close()
{
	if(oOpenStream != NULL)
	{
		oOpenStream->close();
		if(oOpenStream->is_open()) throw InputException("waffleoRai_Utils::FileInputStreamer::close","Failed to close stream!");
		delete oOpenStream;
		oOpenStream = NULL;
	}
}

const byte FileInputStreamer::getNextByte()
{
	if(!isOpen()) throw InputException("waffleoRai_Utils::FileInputStreamer::nextByte","Failed to retrieve next byte - stream is not open!");
	int b = oOpenStream->get();
	//if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::nextByte","Failed to retrieve next byte!");
	read++;
	return (byte)b;
}

void DataOutputStreamer::putByte(byte value)
{
	iTarget.addByte(value);
}

void DataOutputStreamer::putUnsignedShort(u16 value)
{
	const int bcount = 2;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian)
	{
	case big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	case little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	}
}

void DataOutputStreamer::putShort(x16 value)
{
	putUnsignedShort((u16)value);
}

void DataOutputStreamer::put24(u32 value)
{
	const int bcount = 3;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian)
	{
	case big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	case little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	}
}

void DataOutputStreamer::putUnsignedInt(u32 value)
{
	const int bcount = 4;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian)
	{
	case big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	case little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	}
}

void DataOutputStreamer::putInt(x32 value)
{
	putUnsignedInt((u32)value);
}

void DataOutputStreamer::putUnsignedLong(u64 value)
{
	const int bcount = 8;
	const int maxshift = (bcount-1) * 8;

	switch(eEndian)
	{
	case big_endian:
		for(int sh = maxshift; sh >= 0; sh-=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	case little_endian:
		for(int sh = 0; sh <= maxshift; sh+=8) iTarget.addByte((byte)((value >> sh) & 0xFF));
		break;
	}

}

void DataOutputStreamer::putLong(x64 value)
{
	putUnsignedLong((u64)value);
}

const bool FileOutputStreamer::isOpen() const
{
	if(oOpenStream == NULL) return false;
	return oOpenStream->is_open();
}

void FileOutputStreamer::open()
{
	close();
	oOpenStream = new ofstream(sFilePath.c_str(), (ofstream::out | ofstream::binary));
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileOutputStreamer::open","Failed to open stream!");
}

void FileOutputStreamer::openForAppending()
{
	close();
	oOpenStream = new ofstream(sFilePath.c_str(), (ofstream::out | ofstream::binary | ofstream::app));
	if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileOutputStreamer::openForAppending","Failed to open stream!");
}

void FileOutputStreamer::close()
{
	if(oOpenStream != NULL)
	{
		oOpenStream->close();
		if(oOpenStream->is_open()) throw InputException("waffleoRai_Utils::FileOutputStreamer::close","Failed to close stream!");
		delete oOpenStream;
		oOpenStream = NULL;
	}
}

const ofstream& FileOutputStreamer::getStreamView() const
{
	if(oOpenStream == NULL) throw OutputException("waffleoRai_Utils::FileOutputStreamer::getStreamView","Stream is not open!");
	return *oOpenStream;
}

const bool FileOutputStreamer::addByte(byte value)
{
	//if(!isOpen()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::addByte","Failed to add byte - stream is not open!");
	if(!isOpen()) return false;
	oOpenStream->put(value);
	//if(oOpenStream->fail()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::addByte","Failed to add byte!");
	return true;
}

const bool FileOutputStreamer::addBytes(const char* data, long datlen)
{
	if(!isOpen()) return false;
	oOpenStream->write(data, datlen);
	return true;
}

DataStreamerSource::~DataStreamerSource(){}
DataOutputTarget::~DataOutputTarget(){}

}

/***************************************************************************//**
@file FileInput.h
@brief C++ convenience functions and types for input streams, particularly file input.

An interface and set of all-virtual wrapper classes for flexible handling
of input streams.

Includes:
+ iostream
+ fstream
+ filesystem

* **WARNING:** The inclusion of `filesystem` requires C++ 17 or newer!

@author Blythe Hospelhorn
@version 1.0.0
@since December 4, 2022

 ******************************************************************************/

#ifndef FILESTREAMER_IN_H_INCLUDED
#define FILESTREAMER_IN_H_INCLUDED

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

/***************************************************************************//**
An `exception` subclass to throw when an error occurs during an input operation.
******************************************************************************/
class WRCU_DLL_API InputException :public exception
{
private:
	const char* sSource;
	const char* sReason;

public:

	/**
	Construct a `InputException` with information regarding the source of the throw and
	reason for throwing.
	@param source String representing the source of the throw. For example, the full name of a method (eg. `mynamespace::myclass::mymethod`).
	May be left `NULL`.
	@param reason String describing the reason for the exception. Is returned by superclass function `what()`. May be left `NULL`.
	*/
	InputException(const char* source, const char* reason) :sSource(source), sReason(reason) {};
	  
	/**
	Get a string describing the source of the exception. This may be the full name of a method or function, or a file/line number.
	@return Source description string. May be `NULL`.
	*/
	const char* getSourceDescription() const { return sSource; }

	/**
	* `exception` superclass **virtual override**

	From `std::exception` documentation at [cplusplus.com] :

	Returns a null terminated character sequence that may be used to identify the exception.
	The particular representation pointed by the returned value is implementation-defined.
	As a virtual function, derived classes may redefine this function so that specific values are returned.
	@return Reason description string. May be `NULL`.
	[cplusplus.com]: https://cplusplus.com/reference/exception/exception/what/ "cplusplus.com"
	*/
	const char* what() const throw() override { return sReason; }
};

/***************************************************************************//**
An interface for wrapping an input stream and accessing it virtually.
Subclasses may have non-virtual methods, but all interface methods should
be virtual to allow subclasses of `DataStreamerSource` to be passed as an
instance of the superclass while keeping the underlying implementation
flexible.

This is really only intended to be used when the flexibility provided by the
virtual interface is neccessary. For most local stream and I/O operations, 
`std::istream` or the like would probably be the better choice for performance.
******************************************************************************/
class WRCU_DLL_API DataStreamerSource
{
public:

	/**
	Fetch the next byte or character in the input stream (size depends upon implementation),
	moving the stream forward 1 position, and return it. Values smaller than `int` are zero-extended, 
	unless a special value. `EOF` indicates the end of the stream.
	@return Next character in stream, or `EOF` if stream has been depleted.
	*/
	virtual const int get() = 0;

	/**
	Fetch the next byte in the input stream, moving the stream forward one byte.
	Unlike `get()`, because the return value is only 8 bits wide, there is no
	clean way to use a special value to denote the end of the stream.
	Thus, it is important to only use `nextByte()` when the available byte
	count is known (eg. via `remaining()` or `streamEnd()`) or use `get()` instead.
	@return Next byte in stream. Return value is undefined when stream is
	depleted, though will usually be `00` or `FF`.
	*/
	virtual const ubyte nextByte() = 0;

	/**
	Explicitly open the stream, allocating necessary resources.
	Some implementations may automatically open the stream upon
	or before construction, in which case calling `open()` should
	do nothing.
	*/
	virtual void open() = 0;

	/**
	Close the stream, releasing all underlying resources.
	*/
	virtual void close() = 0;

	/**
	Get whether the stream is currently open.
	@return `true` if the stream is open, `false` if not.
	*/
	virtual const bool isOpen() const = 0;

	/**
	Reset the stream's character/byte read counter, if
	the operation is available for the given implementation.
	Superclass default implementation simply returns `false`.
	@return `true` if reset was successful, `false`if counter could not
	be reset due to lack of support or stream being closed.
	*/
	virtual const bool resetReadCounter() { return false; }

	/**
	Get whether the number of remaining characters/bytes to the end
	of the full stream can be known at this time. Remaining count
	is more likely to be known if implementation sources its data
	from memory.
	@return `true` if remaining byte count is known, `false` otherwise.
	*/
	virtual const bool remainingToEndKnown() const = 0;

	/**
	Get the number of remaining bytes/characters from the current
	stream position to the end of the stream. If stream end is
	not known, then this value may be undefined, so a check with
	`remainingToEndKnown()` is crucial to ensuring a useful value.
	@return The size, in bytes/characters of the remainder of the
	stream, if known. If unknown, return value is undefined, though
	it will likely be `SIZE_UNKNOWN`.
	*/
	virtual const size_t remaining() const = 0;

	/**
	Get whether the stream has reached its end, if known.
	@return `true` if the stream end has been reached, `false`
	otherwise.
	*/
	virtual const bool streamEnd() const = 0;

	/**
	Get whether this stream supports seek operations.
	Superclass default implementation returns `false`.
	@return `true` if the stream supports seek. `false` if not.
	*/
	virtual const bool isSeekable() const;

	/**
	Set the stream position to the requested absolute position.
	Note that seek may not be supported for some implementations, so
	a call to `isSeekable()` first is advised.
	Superclass default implementation returns `SIZE_UNKNOWN`.
	@param pos Position in stream to seek to.
	@return Current position of stream after seek. If successful, should
	match `pos`.
	*/
	virtual const streampos seek(const streampos pos);

	/**
	Get the current stream position, if tracked.
	@return Current position of stream, if known. `SIZE_UNKNOWN` otherwise.
	*/
	virtual const streampos tell();

	/**
	Retrieve multiple bytes from the stream and copy them to the provided
	memory buffer. Default superclass implementation just calls `get()`
	in a loop, but subclass implementations may be more efficient.
	@param dst Destination memory buffer.
	@param len Number of bytes to retrieve.
	@return Number of bytes actually copied to destination buffer.
	*/
	virtual const size_t nextBytes(ubyte* dst, const size_t len);

	/**
	Destroy this `DataStreamerSource`. Default superclass destructor
	is empty, but should be virtually overridden.
	*/
	virtual ~DataStreamerSource() {}
};

/***************************************************************************//**
An implementation of `DataStreamerSource` for reading files. Wraps `ifstream`,
but the stream is allocated and managed by the class internally. This allows
an instance to store a `path` and file offsets so that it can be reopened
after closure.

The purpose of this class is to provide a virtual interface for file input.
When the flexibility offered by virtual subclasses is not necessary, using
`ifstream` directly is likely to be faster.
******************************************************************************/
class WRCU_DLL_API FileInputStreamer :public DataStreamerSource
{

private:
	const path sFilePath;

	ifstream* oOpenStream;
	size_t read = 0;
	size_t maxsz = SIZE_UNKNOWN; //Field to save file size so don't have to retrieve from OS when remainingBytes() is called.

	streampos start_boundary = 0;
	streampos end_boundary = SIZE_UNKNOWN;

public:

	/**
	Construct a `FileInputStreamer` that references the provided `path`.
	@param path `std::filesystem::path` instance describing the location of the file to be accessed.
	*/
	FileInputStreamer(const path& path) :sFilePath(path), oOpenStream(nullptr) {}

	/**
	Construct a `FileInputStreamer` that uses the provided string as its source file path.
	Assumes ASCII encoding, but any `std::string` that can be used to instantiate a
	`std::filesystem::path` is acceptable.
	@param ascii_path `std::string` instance describing the location of the file to be accessed.
	*/
	FileInputStreamer(const string& ascii_path) :sFilePath(ascii_path), oOpenStream(nullptr) {}

	/**
	Construct a `FileInputStreamer` that uses the provided string as its source file path.
	Assumes ASCII encoding, but any char string that can be used to instantiate a
	`std::filesystem::path` is acceptable.
	@param ascii_path null-terminated char string describing the location of the file to be accessed.
	*/
	FileInputStreamer(const char* ascii_path) :sFilePath(ascii_path), oOpenStream(nullptr) {}

	/**
	Construct a `FileInputStreamer` that uses the provided wide char string as its source file path.
	Assumes UTF-16 encoding, but any char16 string that can be used to instantiate a
	`std::filesystem::path` is acceptable.
	@param utf16_path null-terminated 16-bit wide char string describing the location of the file to be accessed.
	*/
	FileInputStreamer(const char16_t* utf16_path) :sFilePath(utf16_path), oOpenStream(nullptr) {}

	/**
	Construct a `FileInputStreamer` that uses the provided wide char string as its source file path.
	Assumes UTF-32 encoding, but any char32 string that can be used to instantiate a
	`std::filesystem::path` is acceptable.
	@param utf32_path null-terminated 32-bit wide char string describing the location of the file to be accessed.
	*/
	FileInputStreamer(const char32_t* utf32_path) :sFilePath(utf32_path), oOpenStream(nullptr) {}

	/**
	Get the file path associated with this instance.
	@return `std::filesystem::path` instance describing this file reader's source.
	*/
	const path& getPath() const { return sFilePath; }

	/**
	Get the size of the source file associated with this file reader.
	@return The size, in bytes, of the source file.
	*/
	const size_t fileSize() const;

	/**
	Get the absolute position in the total file of the start position of this reader.
	Reader will ignore any data before this position.
	@return This reader's start position, in bytes, from file start.
	*/
	const streampos getStartBoundary() const { return start_boundary; }

	/**
	Get the absolute position in the total file of the end position of this reader.
	This position is relative to the start of the full file, not the start position.
	Reader will ignore any data after this position.
	@return This reader's end position, in bytes, from file start.
	*/
	const streampos getEndBoundary() const { return end_boundary; }

	/**
	Set the absolute position in the total file of the start position of this reader.
	Reader will ignore any data before this position.
	@param position The position in bytes, relative to the start of the full file,
	to set as the start boundary.
	@return The start boundary set for the reader. If successful, return value should
	match the requested value.
	*/
	const streampos setStartBoundary(const streampos position) {
		if (start_boundary >= 0 && start_boundary < end_boundary) {
			start_boundary = position;
		}
		return start_boundary;
	}

	/**
	Set the absolute position in the total file of the end position of this reader.
	This position is relative to the start of the full file, not the start position.
	Reader will ignore any data after this position.
	@param position The position in bytes, relative to the start of the full file,
	to set as the end boundary.
	@return The end boundary set for the reader. If successful, return value should
	match the requested value.
	*/
	const streampos setEndBoundary(const streampos position) {
		if (end_boundary <= maxsz && end_boundary > start_boundary) {
			end_boundary = position;
		}
		return end_boundary;
	}

	const bool isSeekable() const override;
	const streampos tell() override;

	/**
	Request that the stream be set to a specific position. Position is relative
	to the start of the streamed part of the file (ie. the start boundary), not the full file.
	@param pos Position to set stream to, relative to start boundary.
	@return Position, relative to start boundary, that the stream is at upon return. If does
	not match parameter, seek failed.
	@throws InputException If position is invalid.
	*/
	const streampos seek(const streampos pos) override;

	const bool isOpen() const override {
		return (oOpenStream != NULL && oOpenStream->is_open());
	}

	const bool streamEnd() const override;
	const bool remainingToEndKnown() const override;
	const size_t remaining() const override;

	/**
	Get whether stream status is good (calls `good()` on the underlying `ifstream`)
	@return `true` if stream is in good state, `false` if stream is not good, or not open.
	*/
	const bool isGood() const {
		if (oOpenStream == NULL) return false;
		if (!oOpenStream->is_open()) return true;
		return oOpenStream->good();
	}

	/**
	Get whether stream status is bad (calls `bad()` on the underlying `ifstream`)
	@return `true` if stream is in bad state, `false` if stream is not bad, or not open.
	*/
	const bool isBad() const {
		if (oOpenStream == NULL) return false;
		if (!oOpenStream->is_open()) return true;
		return oOpenStream->bad();
	}

	/**
	Get whether stream has a a fail status (calls `fail()` on the underlying `ifstream`)
	@return `true` if stream has fail flag set, `false` if stream is not in fail state, or not open.
	*/
	const bool isFail() const {
		if (oOpenStream == NULL) return false;
		if (!oOpenStream->is_open()) return true;
		return oOpenStream->fail();
	}

	/**
	Get the number of bytes that have been read since the stream was opened, or since the last
	read counter reset.
	@return Current byte read count.
	*/
	const size_t bytesRead() const { return read; }
	const bool resetReadCounter() override { read = 0; return true; }

	/**
	Open the file stream at the start of the file, allocating an `ifstream`
	on the heap for internal use. This `open()` variant automatically sets the
	start boundary to 0 and end boundary to the file size.
	@throws InputException If stream fails to open.
	*/
	void open() override;

	/**
	Open the file stream at the specified offset. The start offset is
	set as the start boundary for the stream, and treated as relative
	position zero.
	@param startOffset Position relative to the start of the full file
	to open file stream at.
	@throws InputException If stream fails to open or start offset is invalid.
	*/
	void open(const streampos startOffset);

	/**
	Open a part of the file as its own subfile stream. The method
	parameters set the start and end boundaries for this stream instance.
	Data outside the subfile is treated as if it does not exist,
	at least unless the stream is closed and reopened with different boundaries.
	This is useful for accessing files in an archive.
	@param startOffset Position relative to the start of the full file
	to open file stream at.
	@param len Size in bytes of the subfile. End boundary is set to the sum of
	the startOffset and len.
	@throws InputException If stream fails to open or either boundary is invalid.
	*/
	void open(const streampos startOffset, const size_t len);

	/**
	Skip a number of bytes in the stream. These bytes are not included
	in the read counter.
	@param skip_amt Number of bytes to skip.
	@return Number of bytes actually skipped.
	*/
	const size_t skip(const streampos skip_amt);

	/**
	Close the stream and free the underlying resources.
	@throws InputException If stream cannot be closed.
	*/
	void close() override;

	/**
	Get a const view of the underlying `ifstream`, if the stream is open.
	Because the `ifstream` is allocated on the heap when the `FileInputStreamer`
	is opened, if this stream is not open, this method throws an exception as
	it cannot return a `null` reference.
	@return Readonly reference to the `ifstream` underlying this stream. 
	@throws InputException If stream is not open.
	*/
	const ifstream& getStreamView() const;

	/**
	Fetch the next byte in the input stream, moving the stream forward one byte.
	Unlike `get()`, because the return value is only 8 bits wide, there is no
	clean way to use a special value to denote the end of the stream.
	Thus, it is important to only use `nextByte()` when the available byte
	count is known (eg. via `remaining()` or `streamEnd()`) or use `get()` instead.
	@return Next byte in stream. Return value is undefined when stream is
	depleted, though will usually be `00` or `FF`.
	@throws InputException If stream is not open.
	*/
	const ubyte nextByte() override;

	/**
	Fetch the next byte or character in the input stream (size depends upon implementation),
	moving the stream forward 1 position, and return it. Values smaller than `int` are zero-extended,
	unless a special value. `EOF` indicates the end of the stream.
	@return Next character in stream, or `EOF` if stream has been depleted.
	@throws InputException If stream is not open.
	*/
	const int get() override;

	/**
	Retrieve multiple bytes from the stream and copy them to the provided
	memory buffer. Default superclass implementation just calls `get()`
	in a loop, but subclass implementations may be more efficient.
	@param dst Destination memory buffer.
	@param len Number of bytes to retrieve.
	@return Number of bytes actually copied to destination buffer.
	@throws InputException If stream is not open.
	*/
	const size_t nextBytes(ubyte* dst, const size_t len) override;

	virtual ~FileInputStreamer() { close(); }

};

/***************************************************************************//**
An implementation of `DataStreamerSource` for reading files. Is intended to wrap
an existing `ifstream` instance. The key difference between `FileInputStreamer`
and `ifstreamer` is that the former allocates and manages its own heaped `ifstream` 
internally, whereas the latter references an externally defined `ifstream`, starting at
wherever position it is upon construction. If the wrapped `ifstream` is read from
outside the `ifstreamer`, the `ifstreamer` will not know.

The purpose of this class is to provide a virtual interface for file input.
When the flexibility offered by virtual subclasses is not necessary, using
`ifstream` directly is likely to be faster.
******************************************************************************/
class WRCU_DLL_API ifstreamer : public DataStreamerSource
{

private:
	ifstream* stream;
	size_t read = 0;

	streampos start_boundary = 0;
	streampos end_boundary = SIZE_UNKNOWN;

	bool close_source_on_close = false;
	bool destroy_source_on_close = false;

public:

	/**
	Construct an `ifstreamer` that wraps the provided `ifstream`.
	Streamer's start boundary will be set to the current position of the
	input stream.
	@param source `ifstream` instance to wrap.
	*/
	ifstreamer(ifstream& source) :stream(&source) {
		if (source.is_open()) start_boundary = source.tellg();
	}

	/**
	Construct an `ifstreamer` that wraps the provided `ifstream`, and
	is limited to a certain number of bytes from the current position,
	at which point it will treat it as the end of the file.
	Streamer's start boundary will be set to the current position of the
	input stream. End boundary will be set to the current position + size_bytes.
	@param source `ifstream` instance to wrap.
	@param size_bytes Length, in bytes, of subfile.
	*/
	ifstreamer(ifstream& source, size_t size_bytes) :stream(&source) {
		if (source.is_open()) start_boundary = source.tellg();
		end_boundary = static_cast<size_t>(start_boundary) + size_bytes;
	}

	/**
	Get the length of the stream if known. This is only known if the end boundary was
	set at some point.
	@return The length of the stream, in bytes.
	*/
	const size_t fileSize() const { return end_boundary != SIZE_UNKNOWN ? static_cast<size_t>(end_boundary - start_boundary) : SIZE_UNKNOWN; }

	const bool isOpen() const override { return stream && stream->is_open(); }
	const bool streamEnd() const override;

	/**
	Get whether stream status is good (calls `good()` on the underlying `ifstream`)
	@return `true` if stream is in good state, `false` if stream is not good, or not open.
	*/
	const bool isGood() const { return stream->good(); }

	/**
	Get whether stream status is bad (calls `bad()` on the underlying `ifstream`)
	@return `true` if stream is in bad state, `false` if stream is not bad, or not open.
	*/
	const bool isBad() const { return stream->bad(); }

	/**
	Get whether stream has a a fail status (calls `fail()` on the underlying `ifstream`)
	@return `true` if stream has fail flag set, `false` if stream is not in fail state, or not open.
	*/
	const bool isFail() const { return stream->fail(); }
	const bool remainingToEndKnown() const override;
	const size_t remaining() const override;

	/**
	Get the number of bytes that have been successfully read
	through this streamer. Bytes read by calling methods `read()` or
	`get()` directly on the wrapped `ifstream` without going through
	this `ifstreamer` are not counted.
	@return Total bytes read.
	*/
	const size_t bytesRead() const { return read; }
	const bool resetReadCounter() override { read = 0; return true; }

	const bool isSeekable() const override;
	const streampos seek(const streampos pos) override;
	const streampos tell() override;

	void open() override;
	void close() override;

	/**
	Skip a number of bytes in the stream. These bytes are not included
	in the read counter.
	@param skip_amt Number of bytes to skip.
	@return Number of bytes actually skipped.
	*/
	void skip(const streampos skip_amt);
	
	/**
	Get a const view of the underlying `ifstream`, if the stream is open.
	@return Readonly reference to the `ifstream` underlying this stream.
	@throws InputException If the stream reference is `null`.
	*/
	const ifstream& getStreamView();

	const int get() override;
	const ubyte nextByte() override;
	const size_t nextBytes(ubyte* dst, const size_t len) override;

	/**
	Set flag for streamer's close behavior. If the close stream on close flag
	is set, then when this streamer's `close()` is called, the `ifstream` referenced,
	if present, will also be closed. If the flag is not set, then the `ifstream` will
	NOT be closed, though the pointer will be set to `null`. 
	@param flag Boolean value to set the close source on close flag.
	*/
	void setCloseStreamOnClose(const bool flag) { close_source_on_close = flag; }

	/**
	Set flag for streamer's close behavior. If the destroy stream on close flag
	is set, then when this streamer's `close()` is called, the `ifstream` referenced,
	if present, will be closed and deleted. If the flag is not set, then the `ifstream` will
	NOT be closed or deleted, though the pointer will be set to `null`.
	@param flag Boolean value to set the destroy source on close flag.
	*/
	void setDestroyStreamOnClose(const bool flag) { destroy_source_on_close = flag; }

	virtual ~ifstreamer() { close(); }

};

/***************************************************************************//**
An implementation of `DataStreamerSource` for reading from a memory buffer.

The purpose of this class is to provide a virtual interface for file/data input.
When the flexibility offered by virtual subclasses is not necessary, reading
from the buffer directly is likely more efficient.
******************************************************************************/
class WRCU_DLL_API MemInputStreamer : public DataStreamerSource
{
private:
	ubyte* data;
	ubyte* data_end; //Bookkeeping, for speed.
	size_t len = 0;

	ubyte* current_pos = NULL;
	bool is_open = false; //Used to mark closure. If closed, it is assumes the referenced data is expired.

public:


	/**
	Construct a `MemInputStreamer` that will read from the specified buffer.
	@param src Pointer to start of the data buffer to stream from.
	@param size Length of data buffer, in bytes.
	*/
	MemInputStreamer(const void* src, const size_t size) :data((ubyte*)src), len(size), data_end(NULL) { data_end = data + len; };

	const int get() override { return (!is_open || current_pos >= data_end) ? EOF : static_cast<int>(*current_pos++) & 0xff; };
	const ubyte nextByte() override { return (!is_open || current_pos >= data_end) ? 0xff : *current_pos++; };
	const size_t nextBytes(ubyte* dst, const size_t len) override;

	const size_t remaining() const override { return is_open ? static_cast<size_t>(data_end - current_pos) : 0; };
	const bool streamEnd() const override { return is_open ? (current_pos >= data_end) : true; };

	void open() override;
	void close() override { is_open = false; current_pos = NULL; data = NULL; data_end = NULL; len = 0; };

	/**
	Skip a number of bytes in the stream. These bytes are not included
	in the read counter.
	@param skip_amt Number of bytes to skip.
	@return Number of bytes actually skipped.
	*/
	void skip(const streampos skip_amt);

	const bool isOpen() const override { return is_open; }
	const bool resetReadCounter() override { return true; }

	const bool remainingToEndKnown() const override { return true; };
	const bool isSeekable() const override { return true; }
	const streampos seek(const streampos pos) override;
	const streampos tell() override;

	virtual ~MemInputStreamer() { close(); }
};

}

#endif

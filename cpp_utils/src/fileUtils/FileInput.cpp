#include "FileInput.h"

using std::filesystem::filesystem_error;

namespace waffleoRai_Utils {
	
	/*----- DataStreamerSource -----*/

	const size_t  DataStreamerSource::nextBytes(ubyte* dst, const size_t len) {
		if (!dst || len <= 0) return 0;
		size_t i;
		ubyte* pos = dst;
		int b = EOF;
		for (i = 0; i < len; i++) {
			b = get();
			if (b == EOF) return static_cast<size_t>(pos - dst);
			*(pos++) = (ubyte)b;
		}

		return static_cast<size_t>(pos - dst);
	}

	const bool DataStreamerSource::isSeekable() const {
		return false;
	}

	const streampos DataStreamerSource::seek(const streampos pos) {
		return SIZE_UNKNOWN;
	}

	const streampos DataStreamerSource::tell() {
		return SIZE_UNKNOWN;
	}

	/*----- FileInputStreamer -----*/

#define FIS_ISOPEN (oOpenStream != NULL && oOpenStream->is_open())

	const size_t FileInputStreamer::fileSize() const {
		//https://stackoverflow.com/questions/5840148/how-can-i-get-a-files-size-in-c
		/*struct stat stat_buf;
		int rc = stat(sFilePath, &stat_buf);
		return rc == 0 ? stat_buf.st_size : -1;*/

		if (end_boundary > 0) return end_boundary - start_boundary;

		try {
			return static_cast<size_t>(std::filesystem::file_size(sFilePath));
		}
		catch (filesystem_error& x) { return SIZE_UNKNOWN; }
	}

	const bool FileInputStreamer::streamEnd() const {
		if (oOpenStream == NULL) return true;
		if (!oOpenStream->is_open()) return true;
		return (oOpenStream->eof() || (oOpenStream->tellg() >= end_boundary));
	}

	const bool FileInputStreamer::remainingToEndKnown() const {
		if (!FIS_ISOPEN) return false;
		if (end_boundary == SIZE_UNKNOWN) return false;
		return true;
	}

	const size_t FileInputStreamer::remaining() const {
		if (!FIS_ISOPEN) return SIZE_UNKNOWN;
		if (end_boundary == SIZE_UNKNOWN) return SIZE_UNKNOWN;
		streampos tell = oOpenStream->tellg();
		if(tell > end_boundary) return SIZE_UNKNOWN;
		size_t diff = static_cast<size_t>(end_boundary - tell);
		return diff;
	}

	const ifstream& FileInputStreamer::getStreamView() const {
		if (oOpenStream == NULL) throw InputException("waffleoRai_Utils::FileInputStreamer::getStreamView", "Stream is not open!");
		return *oOpenStream;
	}

	void FileInputStreamer::open() {
		//Maybe set exception throwing instead of doing active check?
		if (FIS_ISOPEN) return;
		read = 0;
		end_boundary = maxsz = fileSize();
		oOpenStream = new ifstream(sFilePath, ifstream::in | ifstream::binary);
		if (oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open", "Failed to open stream!");
	}

	void FileInputStreamer::open(const streampos startOffset) {
		if (FIS_ISOPEN) return;
		read = 0;
		end_boundary = maxsz = fileSize();
		if (startOffset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open", "Open offset after end of file!");
		oOpenStream = new ifstream(sFilePath, ifstream::in | ifstream::binary);
		oOpenStream->seekg(startOffset);
		if (oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open", "Failed to open stream!");
		start_boundary = startOffset;
	}

	void FileInputStreamer::open(const streampos startOffset, const size_t len) {
		if (FIS_ISOPEN) return;
		read = 0;
		maxsz = fileSize();
		if (startOffset > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open", "Open offset after end of file!");
		end_boundary = static_cast<uint64_t>(startOffset) + len;
		if (end_boundary > maxsz) throw InputException("waffleoRai_Utils::FileInputStreamer::open", "End offset after end of file!");
		start_boundary = startOffset;
		oOpenStream = new ifstream(sFilePath, ifstream::in | ifstream::binary);
		oOpenStream->seekg(startOffset);
		if (oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::open", "Failed to open stream!");
	}

	const size_t FileInputStreamer::skip(const streampos skip_amt) {
		if (!FIS_ISOPEN) return SIZE_UNKNOWN;
		streampos pos = oOpenStream->tellg();
		streampos trg = pos + skip_amt;
		if (trg > end_boundary) {
			trg = end_boundary;
		}
		oOpenStream->seekg(trg);
		return static_cast<size_t>(oOpenStream->tellg() - pos);
	}

	const bool FileInputStreamer::isSeekable() const { return true; }

	const streampos FileInputStreamer::seek(const streampos pos) {
		if (!FIS_ISOPEN) return SIZE_UNKNOWN;
		streampos trg = pos + start_boundary;
		if (trg > end_boundary) throw InputException("waffleoRai_Utils::FileInputStreamer::seek", "Seek position is invalid!");
		oOpenStream->seekg(trg);
		return oOpenStream->tellg() - start_boundary;
	}

	const streampos FileInputStreamer::tell() {
		if (!FIS_ISOPEN) return SIZE_UNKNOWN;
		return oOpenStream->tellg() - start_boundary;
	}

	void FileInputStreamer::close() {
		if (oOpenStream) {
			oOpenStream->close();
			if (oOpenStream->is_open()) throw InputException("waffleoRai_Utils::FileInputStreamer::close", "Failed to close stream!");
			delete oOpenStream;
			oOpenStream = NULL;
		}
	}

	const ubyte FileInputStreamer::nextByte() {
		if (!FIS_ISOPEN) throw InputException("waffleoRai_Utils::FileInputStreamer::nextByte", "Failed to retrieve next byte - stream is not open!");
		if (oOpenStream->tellg() >= end_boundary) return (ubyte)0xff;
		int b = oOpenStream->get();
		//if(oOpenStream->fail()) throw InputException("waffleoRai_Utils::FileInputStreamer::nextByte","Failed to retrieve next byte!");
		read++;
		return (ubyte)b;
	}

	const int FileInputStreamer::get() {
		if (!FIS_ISOPEN) throw InputException("waffleoRai_Utils::FileInputStreamer::get", "Failed to retrieve next byte - stream is not open!");
		if (oOpenStream->tellg() >= end_boundary) return EOF;
		int b = oOpenStream->get();
		read++;
		return b;
	}

	const size_t FileInputStreamer::nextBytes(ubyte* dst, const size_t len) {
		if (!FIS_ISOPEN) throw InputException("waffleoRai_Utils::FileInputStreamer::nextBytes", "Failed to retrieve data - stream is not open!");
		size_t readamt = len;
		streampos stpos = oOpenStream->tellg();
		streampos trg = stpos + static_cast<streampos>(readamt);
		if (trg > end_boundary) {
			trg = end_boundary;
			readamt = static_cast<size_t>(trg - stpos);
		}
		oOpenStream->read((char*)dst, readamt);
		streampos edpos = oOpenStream->tellg();
		readamt = static_cast<size_t>(edpos - stpos);
		read += readamt;
		return readamt;
	}

	/*----- ifstreamer -----*/

	void ifstreamer::open() {} //Does nothing. It assumes that you passed it an open stream.

	const bool ifstreamer::remainingToEndKnown() const {
		if (stream && end_boundary != SIZE_UNKNOWN) return true;
		return false;
	}

	const size_t ifstreamer::remaining() const {
		if (!stream) return SIZE_UNKNOWN;
		if (end_boundary != SIZE_UNKNOWN) {
			return end_boundary - stream->tellg();
		}
		return SIZE_UNKNOWN;
	}

	const bool ifstreamer::streamEnd() const {
		if (!stream) return true;
		if (end_boundary > SIZE_UNKNOWN) {
			return stream->tellg() >= end_boundary;
		}
		return stream->eof();
	}

	const bool ifstreamer::isSeekable() const { return true; }

	const streampos ifstreamer::seek(const streampos pos) {
		if (!stream) return SIZE_UNKNOWN;
		stream->seekg(start_boundary + pos, std::ios_base::beg);
		return stream->tellg() - start_boundary;
	}

	const streampos ifstreamer::tell() {
		if (!stream) return SIZE_UNKNOWN;
		return stream->tellg() - start_boundary;
	}

	void ifstreamer::skip(const streampos skip_amt) {
		if (!stream) return;
		stream->seekg(skip_amt, std::ios_base::cur);
	}

	void ifstreamer::close() {
		if (!stream) return;
		if (close_source_on_close || destroy_source_on_close) {
			stream->close();
		}
		if (destroy_source_on_close) {
			delete stream;
		}
		stream = NULL;
	}

	const ifstream& ifstreamer::getStreamView() {
		if (!stream) throw InputException("waffleoRai_Utils::ifstreamer::getStreamView", "ifstream reference is null!");
		return *stream;
	}

	const ubyte ifstreamer::nextByte() {
		if (!stream) return 0xff;
		read++;
		return static_cast<ubyte>(stream->get());
	}

	const size_t ifstreamer::nextBytes(ubyte* dst, const size_t len) {
		streampos stpos = stream->tellg();
		stream->read((char*)dst, len);
		size_t read_count = stream->tellg() - stpos;
		read += read_count;
		return read_count;
	}

	const int ifstreamer::get() {
		if (!stream) return EOF;
		read++;
		return stream->get();
	}

	/*----- MemInputStreamer -----*/

	void MemInputStreamer::open() {
		if (is_open | !data) return;
		current_pos = data;
		is_open = true;
	}

	void MemInputStreamer::skip(const streampos skip_amt) {
		if (!is_open || !data) return;
		current_pos += skip_amt;
		if (current_pos > data_end) current_pos = data_end;
	}

	const streampos MemInputStreamer::seek(const streampos pos) {
		if (!is_open || !data) return SIZE_UNKNOWN;
		current_pos = data + pos;
		if(current_pos > data_end) current_pos = data_end;
		if (current_pos < data) current_pos = data;
		return static_cast<size_t>(current_pos - data);
	}

	const streampos MemInputStreamer::tell() {
		if (!current_pos) return SIZE_UNKNOWN;
		return static_cast<streampos>(current_pos - data);
	}

	const size_t MemInputStreamer::nextBytes(ubyte* dst, const size_t len) {
		if (!is_open || !data) return SIZE_UNKNOWN;
		ubyte* startpos = current_pos;
		current_pos += len;
		if (current_pos > data_end) current_pos = data_end;
		size_t cpyamt = static_cast<size_t>(current_pos - startpos);
		memcpy(dst, startpos, cpyamt);
		return cpyamt;
	}

}
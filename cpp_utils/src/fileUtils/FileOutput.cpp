#include "FileOutput.h"

namespace waffleoRai_Utils {

	/*----- FileOutputStreamer -----*/

	const bool FileOutputStreamer::isOpen() const {
		if (oOpenStream == NULL) return false;
		return oOpenStream->is_open();
	}

	void FileOutputStreamer::open() {
		close();
		oOpenStream = new ofstream(sFilePath.c_str(), (ofstream::out | ofstream::binary));
		if (oOpenStream->fail()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::open", "Failed to open stream!");
	}

	void FileOutputStreamer::openForAppending() {
		close();
		oOpenStream = new ofstream(sFilePath.c_str(), (ofstream::out | ofstream::binary | ofstream::app));
		if (oOpenStream->fail()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::openForAppending", "Failed to open stream!");
	}

	void FileOutputStreamer::close() {
		if (oOpenStream != NULL) {
			oOpenStream->close();
			if (oOpenStream->is_open()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::close", "Failed to close stream!");
			delete oOpenStream;
			oOpenStream = NULL;
		}
	}

	const ofstream& FileOutputStreamer::getStreamView() const {
		if (oOpenStream == NULL) throw OutputException("waffleoRai_Utils::FileOutputStreamer::getStreamView", "Stream is not open!");
		return *oOpenStream;
	}

	const bool FileOutputStreamer::addByte(const ubyte value) {
		//if(!isOpen()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::addByte","Failed to add byte - stream is not open!");
		if (!isOpen()) return false;
		oOpenStream->put(value);
		//if(oOpenStream->fail()) throw OutputException("waffleoRai_Utils::FileOutputStreamer::addByte","Failed to add byte!");
		return true;
	}

	const bool FileOutputStreamer::addBytes(const ubyte* data, const size_t datlen) {
		if (!isOpen()) return false;
		oOpenStream->write(reinterpret_cast<const char*>(data), datlen);
		return true;
	}

};
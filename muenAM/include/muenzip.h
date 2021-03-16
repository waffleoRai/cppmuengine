#ifndef MUENZIP_H_INCLUDED
#define MUENZIP_H_INCLUDED

//For compression/decompression streams

#include "FileStreamer.h"

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

class MuenUnzipStream:public DataInputStreamer{

public:
    virtual ~MuenUnzipStream(){}

};

class MuenZipStream{

};

}

#endif // MUENZIP_H_INCLUDED

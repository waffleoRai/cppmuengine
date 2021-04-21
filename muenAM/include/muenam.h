#ifndef MUENAM_H_INCLUDED
#define MUENAM_H_INCLUDED

#include "restree.h"
#include "muenDefs.h"
#include "muenaes.h"
#include "muenzip.h"
#include "muenam_formats.h"

#define MUENCORE_SETBIN_VERSION 1

//64MB
#define MUENCORE_DEFO_COMPBUFF_SIZE 0x4000000

//8GB
#define MUENCORE_DEFO_MAXMEM 0x100000000

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

    const char16_t FNAME_INSTREG[9] = {'i','n','s','t','.','r','e','g','\0'};
    const char16_t FNAME_INIBIN[9] = { 'i','n','i','t','.','b','i','n','\0' };
    const char16_t FNAME_INICFG[9] = { 'i','n','i','t','.','c','f','g','\0' };

    enum e_cardloading_model :uint16_t {

        ALL_ON_BOOT = 0,
        BY_GROUP = 1,
        SCAN_ALL_ASSH = 2

    };

class ResourceHandle{

private:
    ResourceHandle* qprev;
    ResourceHandle* qnext;

public:
    ResourceHandle():qprev(nullptr),qnext(nullptr){}

    const bool appendAfter(ResourceHandle* other);
    const bool removeFromQueue();
    virtual const bool freeResource() = 0;
    virtual const bool isFreeable() = 0; //This way modules handling this type can mark it as still in use (at some point this needs to be made threadsafe)
    virtual const size_t resourceSize() = 0;

    virtual ~ResourceHandle(){}
};

class MuenDexorStream:public DataStreamerSource{

private:
    DataStreamerSource& input;

    ubyte xorkey[16];
    int pos;

    bool delsrc_on_close;
    bool is_open;

public:
    MuenDexorStream(DataStreamerSource& src, ubyte* xkey):input(src),pos(0),delsrc_on_close(false),is_open(false){
        memcpy(xorkey, xkey, 16);
    }

    const ubyte nextByte() override;
	const size_t remaining() const override;
	const bool streamEnd() const override;

	void open() override;
	void close() override;
	const bool isOpen() const override{return is_open;}

	const bool deleteSourceOnClose() const{return delsrc_on_close;}
	void setDeleteSourceOnClose(bool flag){delsrc_on_close = flag;}

    virtual ~MuenDexorStream(){close();}

};

class MuenXorStream:public DataOutputTarget{

private:
    DataOutputTarget& output;

    ubyte xorkey[16];
    int pos;

    bool deltrg_on_close;
    bool is_open;

public:
    MuenXorStream(DataOutputTarget& trg, ubyte* xkey):output(trg),pos(0),deltrg_on_close(false),is_open(false){
        memcpy(xorkey, xkey, 16);
    }

    const bool addByte(ubyte b) override;
	const bool addBytes(const ubyte* data, const size_t datlen) override;
	const bool isOpen() const override{return is_open;}

	void open() override;
    void close() override;

    virtual ~MuenXorStream(){close();}
};

class AssetManager{

private:
    bool is_mutable; //Should be FALSE for play and true for building

    map<string, string> settings;
    map<uint32_t, string> group_paths; //Only applicable if ASSH model is load by group
    map<uint32_t, UnicodeString> group_names; //Empty if not in build mode
    PathTable pathtbl;
    ResourceMap res_map;

    map<ResourceKey,ResourceHandle*> loaded_res;
    ResourceHandle* res_free_queue = nullptr;

    char gamecode[9];
    uint64_t timestamp = 0L;
    uint16_t ver_maj = 0;
    uint16_t ver_min = 0;
    uint16_t ver_bld = 0;

    e_cardloading_model cloadmdl = BY_GROUP;
    bool encrypt_assh = true;
    bool encrypt_all = false;

    ubyte aes_key[16];
    aes_key128_t* active_key = nullptr;
    ubyte hmac_key[16];

    size_t comp_buff_size = MUENCORE_DEFO_COMPBUFF_SIZE; //Size of comp/decomp buffer. Defaults, but should be settable in settings
    uint64_t mem_usage = 0L;
    uint64_t max_mem;

public:
    AssetManager(const uint64_t maxmem, const bool readonly):settings(),res_map(),
        max_mem(maxmem),is_mutable(readonly),group_paths(),loaded_res(), pathtbl(16){
            memset(gamecode, 0, 8);
            memset(aes_key, 0, 16);
            memset(hmac_key, 0, 16);
        }

    AssetManager(const UnicodeString& inibin_path, const bool readonly);

    void setRootPath(UnicodeString& path) { pathtbl.setBasePath(path); }

    DataInputStreamer& openResource(const ResourceKey& key);

    const uint64_t getMaxMemUsage() const { return max_mem;}
    const uint64_t getRecordedMemUsage() const { return mem_usage; }
    void increaseRecordedMemUsage(const uint64_t amt) { mem_usage += amt; }
    void decreaseRecordedMemUsage(const uint64_t amt) { mem_usage -= amt; }

    const string& getSetting(const string& key);
    void setSetting(const string& key, const string& value);

    const bool loadASSH(const string& path);
    const bool loadConfigSettings(const UnicodeString& path);
    const bool saveConfigSettings(const string& path);
    const bool saveMainSettings(const string& path); //Returns false if manager is not mutable

};

class EngineInitFailedException:public exception
{
private:
	const char* sSource;
	const char* sReason;

public:
    EngineInitFailedException(const char* source, const char* reason):sSource(source),sReason(reason){};
	const char* what() const throw(){return sReason;}
};

}

#endif // MUENAM_H_INCLUDED

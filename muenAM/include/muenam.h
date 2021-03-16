#ifndef MUENAM_H_INCLUDED
#define MUENAM_H_INCLUDED

#include "restree.h"

#define MUENCORE_SETBIN_VERSION 1

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

class EngineCore{

private:
    static string* install_path;
    static AssetManager* active_manager;

    static const int readSettingsBin();
    static const int readConfigFile();
    static const int loadASSH();

public:
    static const string& getRegPath(); //Path to where inst.reg should be
    static const string& getInstallPath(); //Get path to installation dir root

    static const AssetManager& initializeEngine();
    static const bool closeEngine();

};

class MuenDexorStream:public DataInputStreamer{

public:
    virtual ~MuenDexorStream(){}

};

class MuenXorStream{

};

class AssetManager{

private:
    bool is_mutable; //Should be FALSE for play and true for building

    map<string, string> settings;
    ResourceMap res_map;

    char[8] gamecode;
    uint64_t timestamp;
    uint16_t ver_maj;
    uint16_t ver_min;
    uint16_t ver_bld;

    bool master_assh;
    bool encrypt_assh;
    bool encrypt_all;

    byte aes_key[16];
    byte hmac_key[16];

    uint64_t mem_usage;
    uint64_t max_mem;

public:
    AssetManager(const uint64_t maxmem, const bool readonly):settings(),res_map(),mem_usage(0L),max_mem(maxmem),is_mutable(readonly){}

    DataInputStreamer& openResource(const ResourceKey& key);

    const uint64_t getMaxMemUsage() const;
    const uint64_t getRecordedMemUsage() const;
    void increaseRecordedMemUsage(const uint64_t amt);
    void decreaseRecordedMemUsage(const uint64_t amt);

    const string& getSetting(const string& key);
    void setSetting(const string& key, const string& value);

    const bool loadASSH(const string& path);
    const bool saveConfigSettings(const string& path);
    const bool saveMainSettings(const string& path); //Returns false if manager is not mutable

};

}

#endif // MUENAM_H_INCLUDED

#include "muencore.h"

#define INITEN_ERR_NONE 0
#define INITEN_ERR_NOREG 1
#define INITEN_ERR_NOIDMATCH 2
#define INITEN_ERR_INIBIN_READ_FAIL 3
#define INITEN_ERR_INICFG_READ_FAIL 4

namespace waffleoRai_muengine{

/*----- EngineCore -----*/

    string EngineCore::reg_path = MUEN_DEFO_INIREGDIR;
    char EngineCore::progid[9] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x','\0'};
    UnicodeString EngineCore::install_path = "";
    AssetManager* EngineCore::active_manager = nullptr;

const int EngineCore::readSettingsBin() {
    try {
        active_manager = new AssetManager(install_path + FILE_SEP16 + FNAME_INIBIN, true);
        active_manager->setRootPath(install_path);
    }
    catch (exception& x) {
        return INITEN_ERR_INIBIN_READ_FAIL;
    }
    return INITEN_ERR_NONE;
}

const int EngineCore::readConfigFile() {
    if(!active_manager) return INITEN_ERR_INICFG_READ_FAIL;
    try {
        if(!active_manager->loadConfigSettings(install_path + FILE_SEP16 + FNAME_INICFG)) return INITEN_ERR_INICFG_READ_FAIL;
    }
    catch (exception& x) {
        return INITEN_ERR_INICFG_READ_FAIL;
    }

    return INITEN_ERR_NONE;
}

const int EngineCore::loadASSH() {
     //TODO
    return INITEN_ERR_NONE;
}

const int EngineCore::readInstallPathFromReg(){
    string instreg_path = reg_path + FILE_SEP + "inst.reg";

    try {
        FileInputStreamer fis = FileInputStreamer(instreg_path);
        DataInputStreamer dis = DataInputStreamer(fis, Endianness::little_endian);

        char codebuff[9];
        memset(codebuff, 0, 9);
        uint16_t strlen = 0;
        int cmp = 0;

        while (!dis.streamEnd()) {
            dis.nextBytes(reinterpret_cast<ubyte*>(codebuff), 8);
            cmp = strcmp(codebuff, progid);
            if (cmp != 0) {
                //No match. Skip next string.
                strlen = dis.nextUnsignedShort();
                if (strlen % 2) strlen++;
                dis.skip(strlen);
            }
            else {
                //Match. Load in UTF8 string
                strlen = dis.nextUnsignedShort();
                dis.readUTF8String(install_path, static_cast<size_t>(strlen));

                dis.close();
                return INITEN_ERR_NONE;
            }
        }
        dis.close();
    }
    catch (exception& ex) {
        return INITEN_ERR_NOREG;
    }

    return INITEN_ERR_NOIDMATCH;
}

const uint64_t EngineCore::getProgramID_u64() {
    uint64_t* ptrn = reinterpret_cast<uint64_t*>(progid);
    return *ptrn;
}

const UnicodeString& EngineCore::getInstallPath() {
    if (install_path.isEmpty()) {
        //Try to load
        readInstallPathFromReg();
    }
    return install_path;
}

void EngineCore::setProgramID(const char* id) {
    int i = 0;
    while (i < 8 && (id[i] != '\0')) {
        if (id[i] >= 0x20 && id[i] < 0x7F) progid[i] = id[i];
        else progid[i] = '?';
        i++;
    }
    while (i < 8) progid[i++] = '0';
}

void EngineCore::setProgramID(const uint64_t id) {
    const char* cptr = reinterpret_cast<const char*>(&id);
    int i = 0;
    for (i = 0; i < 8; i++) {
        if (cptr[i] >= 0x20 && cptr[i] < 0x7F) progid[i] = cptr[i];
        else progid[i] = '?';
    }
}

void EngineCore::setProgramID(const string& id) {
    int i = 0;
    size_t strlen = id.length();
    while (i < strlen) {
        if (id[i] >= 0x20 && id[i] < 0x7F) progid[i] = id[i];
        else progid[i] = '?';
        i++;
    }
    while (i < 8) progid[i++] = '0';
}

AssetManager& EngineCore::initializeEngine() {
    //TODO
    //Private method, by the time this is called, install path should have been set

    //Load settings bin
    int res = readSettingsBin();
    if(res == INITEN_ERR_INIBIN_READ_FAIL) throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", "Init settings file could not be read!");

    //Load init values
    res = readConfigFile();
    if (res == INITEN_ERR_INICFG_READ_FAIL) throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", "Init config file could not be read!");

    //Load any initial asset pack headers (like if there is a master...)

    //Return manager
    if (!active_manager) throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", "Unknown error: manager not established!");
    return *active_manager;
}

AssetManager& EngineCore::initializeEngine(const string& program_id) {
    return initializeEngine(program_id, nullptr);
}

AssetManager& EngineCore::initializeEngine(const string& program_id, const char* regpath) {
    setProgramID(program_id);
    reg_path = string(regpath);
    int res = readInstallPathFromReg();
    string reason;
    switch (res) {
        case INITEN_ERR_NOREG: 
            reason = "No init.reg file at \"";
            reason += regpath;
            reason += "\"";
            throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", reason.c_str());
        case INITEN_ERR_NOIDMATCH: 
            reason = "init.reg file at \"";
            reason += regpath;
            reason += "\" did not contain record for id ";
            reason += program_id;
            throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", reason.c_str());
    }
    return initializeEngine();
}

AssetManager& EngineCore::initializeEngine(const uint64_t program_id) {
    return initializeEngine(program_id, nullptr);
}

AssetManager& EngineCore::initializeEngine(const uint64_t program_id, const char* regpath) {
    setProgramID(program_id);
    reg_path = string(regpath);
    int res = readInstallPathFromReg();
    string reason;
    switch (res) {
    case INITEN_ERR_NOREG:
        reason = "No init.reg file at \"";
        reason += regpath;
        reason += "\"";
        throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", reason.c_str());
    case INITEN_ERR_NOIDMATCH:
        reason = "init.reg file at \"";
        reason += regpath;
        reason += "\" did not contain record for id ";
        reason += program_id;
        throw EngineInitFailedException("waffleoRai_muengine::EngineCore::initializeEngine", reason.c_str());
    }
    return initializeEngine();
}

AssetManager& EngineCore::initializeEngine(const char16_t* installpath) {
    install_path = UnicodeString(installpath);
    initializeEngine();
}

const bool EngineCore::closeEngine() {
    //TODO
    return false;
}

const char EngineCore::getWindowsDrive() {
#ifdef _WIN32
    //This is a sketchy function that operates under the assumption that the drive name is one letter
    //But I don't feel like being fancier
    TCHAR cbuff[512];
    if (!GetSystemDirectory(cbuff, 512)) return '\0';
    if (sizeof(TCHAR) == 1) { return cbuff[0]; }
    //Else assumed to be a wide char
    return (char)cbuff[0];
#else
    return '\0';
#endif
}

}

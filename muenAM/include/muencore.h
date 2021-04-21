#ifndef MUENCORE_H_INCLUDED
#define MUENCORE_H_INCLUDED

#include "muenam.h"

//Defo init.reg path macros

using namespace waffleoRai_Utils;

namespace waffleoRai_muengine{

class EngineCore{

private:
    static string reg_path; //Make this settable by calling program! (So can be put with a game, not alone with engine)

    static char progid[9];
    static UnicodeString install_path; 
    static AssetManager* active_manager;

    static const int readSettingsBin();
    static const int readConfigFile();
    static const int loadASSH();
    static const int readInstallPathFromReg();
    static AssetManager& initializeEngine();

public:
    static const char* getProgramID(){return progid;}
    static const uint64_t getProgramID_u64();
    static const string& getRegPath(){return reg_path;} //Path to where inst.reg should be
    static const UnicodeString& getInstallPath(); //Get path to installation dir root

    static const char getWindowsDrive();

    static void setProgramID(const char* id);
    static void setProgramID(const string& id);
    static void setProgramID(const uint64_t id);
    static void setRegPath(const string& path){reg_path = path;} //Should copy it??
    static void setInstallPath(const UnicodeString& path){install_path = path;}

    static AssetManager& initializeEngine(const string& program_id);
    static AssetManager& initializeEngine(const string& program_id, const char* regpath);
    static AssetManager& initializeEngine(const uint64_t program_id);
    static AssetManager& initializeEngine(const uint64_t program_id, const char* regpath);
    static AssetManager& initializeEngine(const char16_t* installpath);
    static const bool closeEngine();

};
}


#endif // MUENCORE_H_INCLUDED

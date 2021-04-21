
#include "muenam.h"

namespace waffleoRai_muengine{

/*----- MuamDexorStream -----*/

const ubyte MuenDexorStream::nextByte(){
    if(pos >= 16) pos = 0;
    return input.nextByte() ^ xorkey[pos++];
}

const size_t MuenDexorStream::remaining() const{
    return input.remaining() + (16-pos);
}

const bool MuenDexorStream::streamEnd() const{
    return(remaining() <= 0);
}

void MuenDexorStream::open(){
    if(!input.isOpen()) input.open();
    is_open = true;
}

void MuenDexorStream::close(){
    if(!is_open) return;
    if(delsrc_on_close) delete &input;
    is_open = false;
}

/*----- MuamXorStream -----*/

const bool MuenXorStream::addByte(ubyte b){
    if(pos >= 16) pos = 0;
    output.addByte(b ^ xorkey[pos++]);
    return true;
}

const bool MuenXorStream::addBytes(const ubyte* data, const size_t datlen){
    if(!data || datlen <= 0) return false;
    size_t i = 0;
    const ubyte* ptr = data;
    for(i = 0; i < datlen; i++) addByte(*ptr++);
    return true;
}

void MuenXorStream::open(){
    if(!output.isOpen()) output.open();
    is_open = true;
}

void MuenXorStream::close(){
    if(!is_open) return;
    if(deltrg_on_close) delete &output;
    is_open = false;
}

/*----- ResourceHandle -----*/

/*----- AssetManager -----*/

AssetManager::AssetManager(const UnicodeString& inibin_path, const bool readonly){

    is_mutable = readonly;
    settings = map<string, string>();
    group_paths = map<uint32_t, string>();
    group_names = map<uint32_t, UnicodeString>();
    res_map = ResourceMap();

    FileInputStreamer fis = FileInputStreamer(inibin_path.getBuffer());
    DataInputStreamer dis = DataInputStreamer(fis, Endianness::little_endian);

    //Read in the header (and byte reverse if system is BE)
    muen_initbin_hdr_t hdr;
    dis.nextBytes(reinterpret_cast<ubyte*>(&hdr), sizeof(muen_initbin_hdr_t));
    if (wrcu_sys_big_endian()) brev_muen_initbin_hdr(&hdr);

    //Copy stuff from header
    max_mem = hdr.memlmt;
    memcpy(gamecode, hdr.gamecode, 8);
    gamecode[8] = '\0';
    timestamp = hdr.last_mod;
    ver_maj = hdr.gamever_maj;
    ver_min = hdr.gamever_min;
    ver_bld = hdr.gamever_bld;
    memcpy(aes_key, hdr.aeskey, 16);
    memcpy(hmac_key, hdr.hmackey, 16);
    encrypt_assh = (hdr.flags & INIBIN_HDR_FLAG_ASSHAES);
    encrypt_all = (hdr.flags & INIBIN_HDR_FLAG_ALLAES);
    cloadmdl = static_cast<e_cardloading_model>(hdr.flags & 0x3);

    //Path Table
    //If windows, flip slashes as they are read in
    int32_t pcount = dis.nextInt();
    if (pcount > 0) {
        pathtbl = PathTable(pcount);
        size_t s = static_cast<size_t>(pcount);
        s <<= 2;
        dis.skip(s); //Skip the ptr table

        int32_t i = 0;
        uint16_t strlen = 0;
        int j = 0;
        for (i = 0; i < pcount; i++) {
            //Read in VLS ASCII strings.
            strlen = dis.nextUnsignedShort();
#           ifdef _WIN32
            char* buff = (char*)malloc(strlen+1);
            for (j = 0; j < strlen; j++) {
                const char c = static_cast<const char>(dis.nextByte());
                if (c == '/') buff[j] = '\\';
                else buff[j] = c;
            }
            buff[strlen] = '\0';
            pathtbl.addPath(buff);
            free(buff);
#           else
            string sbuff = string();
            sbuff.reserve(strlen + 1);
            dis.readASCIIString(sbuff, strlen+1);
            pathtbl.addPath(sbuff);
#           endif

            if (strlen % 2) dis.nextByte();
        }

    }
    else {
        pathtbl = PathTable(16);
    }
   

    //Group Table
    if (!dis.streamEnd() && cloadmdl == BY_GROUP) {
        pcount = dis.nextInt();
        if (pcount > 0) {
            int32_t i = 0;
            uint32_t gid = 0;
            uint16_t strlen = 0;
            int j = 0;
            for (i = 0; i < pcount; i++) {
                //Get group ID
                gid = dis.nextUnsignedInt();
                //Read in VLS ASCII strings.
                strlen = dis.nextUnsignedShort();
#           ifdef _WIN32
                char* buff = (char*)malloc(strlen + 1);
                for (j = 0; j < strlen; j++) {
                    const char c = static_cast<const char>(dis.nextByte());
                    if (c == '/') buff[j] = '\\';
                    else buff[j] = c;
                }
                buff[strlen] = '\0';
                group_paths[gid] = buff;
                free(buff);
#           else
                string sbuff = string();
                sbuff.reserve(strlen + 1);
                dis.readASCIIString(sbuff, strlen + 1);
                group_paths[gid] = sbuff;
#           endif

                if (strlen % 2) dis.nextByte();
            }
        }
    }

    dis.close();
}

const bool AssetManager::loadConfigSettings(const UnicodeString& path) {
    //Just a text (ASCII) file...
    try {
        FileInputStreamer fis = FileInputStreamer(path.getBuffer());
        char buff[1024]; 
        int i = 0;
        bool kmode = true;
        string key, val;
        while (!fis.streamEnd()) {
            const char c = fis.nextByte();
            if ((c == '=' || i >= 1023) && kmode) {
                //End of key.
                buff[i] = '\0';
                key = string(buff);
                i = 0;
                kmode = false;
            }
            else if (c == '\n' || i >= 1023) {
                //End of value or key without value
                buff[i] = '\0';
                if (kmode) {
                    key = string(buff);
                    settings[key] = "true";
                }
                else {
                    val = string(buff);
                    settings[key] = val;
                }
                i = 0;
                kmode = true;
            }
            else {
                //Just copy into buffer.
                //Ignoring those damn carriage returns
                if (c != '\r') buff[i++] = c;
            }
        }

        if (i > 0) {
            //There's a last line not terminated by a newline. Save.
            buff[i] = '\0';
            if (kmode) {
                key = string(buff);
                settings[key] = "true";
            }
            else {
                val = string(buff);
                settings[key] = val;
            }
        }

        fis.close();
    }
    catch (exception& ex) {
        printf(ex.what());
        return false;
    }
    
    return true;
}

}

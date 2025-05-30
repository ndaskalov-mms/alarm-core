#pragma once
#include <string.h>

#define lprintf			printf
extern int ErrWrite(int err_code, const char* what, ...);//
extern char prnBuf[1024];
// --------------- error codes -----------------//
//enum errorID {
//    ERR_INFO = 2,                             // just print if INFO is set
//    ERR_OK = 0,                           	// no error
//    ERR_CRITICAL = -1,                        // critical error occured, 
//    ERR_DEBUG = -2, 							// debug print, can be enabled/disable by #define DEBUG
//    ERR_WARNING = -3, 						// warning print  can be enabled/disable by #define WARNING
//    ERR_DB_INDEX_NOT_FND = -10,				// cannot find error in the database
//};

#ifndef ARDUINO
typedef  FILE* IOptr;
IOptr VS_filehandle;
#endif
#ifdef ARDUINO
typedef File IOptr;
IOptr LittleFS_filehandle;
#endif
//
extern int parseConfigFile(char buffer[], int bufferLen, int saveFlag);
//extern int alrmConfig2Json(IOptr stream);
//
// calculate 8-bit CRC
byte crc8(const byte* addr, int len)
{
    byte crc = 0;
    while (len--)
    {
        byte inbyte = *addr++;
        for (byte i = 8; i; i--)
        {
            byte mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix)
                crc ^= 0x8C;
            inbyte >>= 1;
        }  // end of for
    }  // end of while
    return crc;
}  // end of crc8
//
// init storage
// return: true on succsess WARNING - if storage is not formatted, formats it
//         fail on failure
//
int storageSetup() {
    //
    lprintf("Inizializing FS...\n");
#ifdef ARDUINO
    log_i("Begin setup\n");
    //logger.println("Inizializing FS...");
    if (LittleFS.begin(true)) {   // TODO FORMAT_LittleFS_IF_FAILED
        log_i("Storage init success\n");
        return true;
    }
    else {
        log_e("Storage init fail\n");
        return false;
    }
#endif
    return true;
}
//
// close storage
// return: true on succsess WARNING - if storage is not formatted, formats it
//         fail on failure
//
int storageClose() {
    LOG_INFO("Shutting down storage\n");
#ifdef ARDUINO
    LittleFS.end();
    LOG_INFO("Shutting down storage\n");
#endif
    return true;

}
//
int formatStorage() {
    LOG_INFO("Formatting file system\n");
    return true;
#ifdef ARDUINO
    LOG_INFO("Formatting file system  ");
    if (LittleFS.format()) {
        LOG_INFO("-  DONE\n");
        return true;
    }
    else {
        LOG_INFO("-  FAIL!!!\n");
        return false;
    }
#endif
}
//
void printAlarmConfig(byte* cBuf) {
    lprintf("Not implemented\n");
    return;
}
//
// Remove file from FS, returns 0 on succsess
//
int alarmFileDelete(const char cFileName[]) {
    int res;
#ifndef ARDUINO
    int i = 0;
    if (cFileName[i] == '/') {                          // in LittleFS all file names start with /
        i++;
    }
    res = remove(&cFileName[i]);            
#endif
#ifdef ARDUINO
    res = LittleFS.remove(cFileName);
#endif
    if (res) 
        LOG_CRITICAL("Unable to remove file %s", cFileName);
    return res;
}
//
//
IOptr alarmFileOpen(const char cFileName[], const char* mode) {
    IOptr filehandle; 
    //
#ifndef ARDUINO
    if ((strlen(masterDataPrefix) + strlen(cFileName)) > sizeof(prnBuf) - 1) {
        LOG_CRITICAL("Too long data prefix or file name");             // cannot print names as buffer is detected as small
        return 0;
    }
    //strcpy(prnBuf, masterDataPrefix);
    //strcpy(&prnBuf[strlen(prnBuf)], cFileName);
    snprintf(prnBuf, sizeof(prnBuf), "%s%s", masterDataPrefix, cFileName);
    printf("Full file name to open: %s\n", prnBuf);
    //
    FILE* tmpFile = nullptr;
    if (fopen_s(&tmpFile, prnBuf, mode) == 0) {
        filehandle = tmpFile;
    }
    else {
        lprintf("Unable to open file!\n");
        filehandle = nullptr;
    }
    return filehandle;
#endif
#ifdef ARDUINO
    filehandle = LittleFS.open(cFileName, mode);
    if (!filehandle || filehandle.isDirectory()) {
        LOG_CRITICAL("Failed to open file %s for reading\n", cFileName);
        return (IOptr) 0;                               // to handle directory case
    }
    return filehandle;
#endif
}
//
// returns 0 on succsess, otherwise error code
//
int alarmFileClose(IOptr filehandle) {
#ifndef ARDUINO
    int ret = fclose(filehandle);
    return ret;
#endif
#ifdef ARDUINO
    filehandle.close();
    return false;
#endif
}
//
// 
//
int alarmFileRead(byte* bufferptr, size_t size, IOptr stream) {
#ifndef ARDUINO
    int i = fread((void*)bufferptr, sizeof(byte), size, stream);
    return (i);
#endif
#ifdef ARDUINO
    //return (stream.read((byte*)&tmpConfig, sizeof(tmpConfig)));
    return (stream.read(bufferptr, size));
#endif
}
//
//  returns true if all bytes of the buffer written successfully
//
int alarmFileWrite(const byte* bufferptr, int size, IOptr stream) {
    int cnt;
#ifndef ARDUINO
    cnt = fwrite((void*)bufferptr, sizeof(byte), size, stream);
#endif
#ifdef ARDUINO
    cnt = stream.write(bufferptr, size);
    if (cnt != size)
        lprintf("File write fail: requested %d reported %d", size, cnt);
#endif
    return(cnt == size);
}
//
// Rename file from FS, returns 0 on succsess
//
int alarmFileRename(const char oldName[], const char newName[]) {
    int res;     
#ifndef ARDUINO
    int offNew = 0;     int offOld = 0;
    if (oldName[offOld] == '/')
        offOld++;
    if (newName[offNew] == '/')
        offNew++;
    res = !rename(&oldName[offOld], &newName[offNew]);
#endif
#ifdef ARDUINO
    res = LittleFS.rename(oldName, newName);
#endif
    if (!res) 
        LOG_CRITICAL("Unable to rename file %s to %s\n", oldName, newName);
    else
        LOG_CRITICAL("Successfully renamed file %s to %s\n", oldName, newName);
    return res;
}
//
//  returns 0 if renamed successfully
//
int renameFile(const char oldName[], const char newName[]) {
    int res;
    lprintf("Renaming config file %s to %s\n", oldName, newName);

    if (IOptr cFile = alarmFileOpen(newName, "r")) {
        if (alarmFileClose(cFile)) {
            lprintf("Failed to check if backup config file exists\n");
            return -1;
        }
        res = alarmFileDelete(newName);
        if (res) {
            lprintf("Failed remove backup config file size\n");
            return -1;
        }
    }
    res = alarmFileRename(oldName, newName);
    return(res);
}
//
//  
//
int alarmFileSize(IOptr stream) {
    long off;
#ifndef ARDUINO
    FILE* fp = stream;
    if (fseek(fp, 0, SEEK_END))
    {
        lprintf("Failed to fseek while determining config file size\n");
        return 0;
    }
    off = ftell(fp);
    if (off == -1)
    {
        lprintf("Failed to ftell while determining config file size\n");
        return 0;
    }
    //lprintf("Config fseek_filesize: %ld\n", off);
    if (fseek(fp, 0, SEEK_SET))
    {
        lprintf("Failed to fseek back to start of file while determining config file size\n");
        return 0;
    }
#endif
#ifdef ARDUINO
    off = stream.size();
#endif
    return (unsigned int)off;
}
//
// Returns 0 in case of error, otherwise lenth of the payload converted
// params:      const char cFileName[]  - file to be read 
//              byte* bufPtr            - buf to read the file content
//              int   bufLen            - max buffer size
//
int loadConfig(const char cFileName[], byte* bufPtr, int bufLen) {
    lprintf("Reading CSV config file %s\n", cFileName);
    IOptr cFile = alarmFileOpen(cFileName, "rb");                 // alarmFopen will return void ptr which 
    if (!cFile) {                                    // in fact is ptr to FILE * or to File depends on platform
        LOG_CRITICAL("Problem opening CSV config file\n");
        return 0;
    }
    int rlen = alarmFileSize(cFile);
    if (rlen >= bufLen) {
        LOG_CRITICAL("Config file size %d larger than buffer size %d\n", rlen, bufLen);
        alarmFileClose(cFile);
        return 0;

    }
    memset((void*)bufPtr, 0, bufLen);
    int alen = alarmFileRead(bufPtr, rlen, cFile);
    if (rlen != alen) {
        LOG_CRITICAL("Problem reading config file file system len %d, read %d\n", rlen, alen);
        alarmFileClose(cFile);
        return 0;
    }
    //
    alarmFileClose(cFile);
    LOG_DEBUG("Content of config file read\n");
    return rlen;
}
//
//
//
int saveCsvConfig(const char cFileName[], const byte* buf, int len) {
#ifdef ARDUINO
    // LittleFS.remove(F("/cFileName"));

#endif
    IOptr cFile;
    // File cFile = LittleFS.open(cFileName, "w");
    // err = fopen_s(&cFile, cFileName, "wb");
    cFile = alarmFileOpen(cFileName, "wb");
    if (cFile) {
        LOG_DEBUG("Writing config file\n");
        //cFile.write((byte*) &alarmConfig, sizeof(alarmConfig));
        //cntwritten = fwrite(&alarmConfig, sizeof(byte), sizeof(alarmConfig), cFile);
        if (!alarmFileWrite(buf, len, cFile)) {
            LOG_CRITICAL("Problem writing config file!\n");
            return false;
        }
        if (alarmFileClose(cFile)) {
            LOG_CRITICAL("Problem on closing config file!\n");
            return false;
        }
        return true;
    }
    else {
        LOG_CRITICAL("Problem opening config file\n");
        return false;
    }
}
//
//  int alrmConfig2Json(struct CONFIG_t* alarmCfg, FILE* stream) - convert and store alarmConfig struct to JSON
//  params:     const char cFileName[] - filename
//  returns:    true on success, otherwise false
//
//int saveJsonConfig(const char cFileName[]) {
//#ifdef ARDUINO
//    // LittleFS.remove(F("/cFileName"));
//#endif
//    IOptr cFile;
//    cFile = alarmFileOpen(cFileName, "w");
//    if (cFile) {
//        LOG_DEBUG("Writing JSON config file\n");
//        //
//        if (!alrmConfig2Json(cFile)) {
//            LOG_CRITICAL("Problem writing JSON config file!\n");
//            return false;
//        }
//        //
//        if (alarmFileClose(cFile)) {
//            LOG_CRITICAL("Problem on closing JSON config file!\n");
//            return false;
//        }
//        return true;
//    }
//    else {
//        LOG_CRITICAL("Problem opening JSON config file\n");
//        return false;
//    }
//}
//
// TODO - rework to use array of pointer:len entries. Entries shall include at least: version, all items in dbPtrArr[], csum
// then save/read chunks of len size from/to pointer
//
//int readBinConfig(const char cFileName[], struct CONFIG_t *cfgPtr) {
//    return 1;
//    IOptr cFile;
//    //err = fopen_s(&cFile, cFileName, "rb");
//    cFile = alarmFileOpen(cFileName, "rb");
//    memset((void*)cfgPtr, 0, sizeof(struct CONFIG_t));		// clear tmp buffer
//    if (cFile) {
//        // numread = fread_s( buffer_ptr, BUFFERSIZE, ELEMENTSIZE, ELEMENTCOUNT, stream )
//        // int rlen = cFile.read((byte*) &tmpConfig, sizeof(tmpConfig))
//        // int rlen = fread_s((byte*)&tmpConfig, sizeof(tmpConfig), sizeof(byte), sizeof(tmpConfig), cFile);
//        //
//        int rlen = alarmFileRead((byte*)cfgPtr, sizeof(struct CONFIG_t), cFile);
//        if (rlen != sizeof(struct CONFIG_t)) {
//            ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong len!\n");
//            alarmFileClose(cFile);
//            return false;
//        }
//        //
//        alarmFileClose(cFile);
//        //fclose(cFile);
//        byte cs8;
//        cs8 = crc8((byte*)(byte*)cfgPtr, sizeof(struct CONFIG_t) - 1);
//        //cs8 += 1;                         // intentional error        
//        if (cs8 != cfgPtr->csum) {
//            ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong csum!\n");
//            return false;
//        }
//        if (SW_VERSION != cfgPtr->version) {
//            ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong SW version!\n");
//            return false;
//        }
//        lprintf("Content of config file read\n");
//        //printAlarmConfig((byte*) &tmpConfig);
//        //memcpy((byte*)&alarmConfig, (byte*)&tmpConfig, sizeof(alarmConfig));
//        //lprintf("Content of configured DB\n");
//        //printAlarmConfig((byte*) &alarmConfig);
//        ErrWrite(ERR_DEBUG, "Reading config file done\n");
//        return true;
//    }
//    else {
//        ErrWrite(ERR_CRITICAL, "Problem opening BIN config file\n");
//        return false;
//    }
//}
//
//
//int saveBinConfig(const char cFileName[]) {
//#ifdef ARDUINO
//    // LittleFS.remove(F("/cFileName"));
//#endif
//   //  calc 8 bit checksum first
//   alarmConfig.csum = crc8((byte*)&alarmConfig, sizeof(alarmConfig) - 1);
//   //    
//   IOptr cFile;
//   // File cFile = LittleFS.open(cFileName, "w");
//   // err = fopen_s(&cFile, cFileName, "wb");
//   cFile = alarmFileOpen(cFileName, "wb");
//   if (cFile) {
//       ErrWrite(ERR_DEBUG, "Writing config file\n");
//       //cFile.write((byte*) &alarmConfig, sizeof(alarmConfig));
//       //cntwritten = fwrite(&alarmConfig, sizeof(byte), sizeof(alarmConfig), cFile);
//       if(!alarmFileWrite((byte *)&alarmConfig, sizeof(alarmConfig), cFile)) {
//           ErrWrite(ERR_CRITICAL, "Problem on create config file!\n");
//           return false;
//       }
//       if (alarmFileClose(cFile)) {
//           ErrWrite(ERR_CRITICAL, "Problem on closing config file!\n");
//           return false;
//       }
//       /*
//       // readback and verify
//       //cFile = SPIFFS.open(cFileName, "r");
//       //err = fopen_s(&cFile, cFileName, "rb");
//       cFile = alarmFileOpen(cFileName, "rb");
//       if (!cFile) {
//           ErrWrite(ERR_DEBUG, "Reading back config file\n");
//           //int rlen = cFile.read((byte*) &tmpConfig, sizeof(tmpConfig)) != sizeof(tmpConfig))
//           //int rlen = fread_s((byte*)&tmpConfig, sizeof(tmpConfig), sizeof(byte), sizeof(tmpConfig), cFile);
//           int rlen = alarmFileRead((byte*)&tmpConfig, sizeof(tmpConfig), cFile); 
//           if (rlen != sizeof(tmpConfig))
//               ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong len!\n");
//           //Serial.printf("Problem on reading config file! Read %d expected %d\n",rlen, sizeof(tmpConfig) );
//           alarmFileClose(cFile);
//           return false;
//       }
//       if (alarmFileClose(cFile)) {
//           ErrWrite(ERR_CRITICAL, "Problem on closing config file after verification!\n");
//           return false;
//       }
//       byte cs8;
//       cs8 = crc8((byte*)&tmpConfig, sizeof(tmpConfig) - 1);
//       //cs8 += 1;                         // intentional error
//       if (cs8 != tmpConfig.csum) {
//           ErrWrite(ERR_CRITICAL, "Problem reading config file - wrong csum!\n");
//           //Serial.printf("Read CS %d differs from calculated %d", tmpConfig.csum, cs8);
//           return false;
//       }
//       */
//       //lprintf("Content of config file written\n");
//       //printAlarmConfig((byte*) &tmpConfig);
//       return true;
//   }
//   else {
//       ErrWrite(ERR_CRITICAL, "Problem opening config file\n");
//       return false;
//   }
//   return true;
//}

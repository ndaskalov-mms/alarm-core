// this is heeded for windows JSON library
//#include "json.hpp"
#include "alarm-core-mqtt.h" // Include the new header
#include "alarm-core.h"
#include "alarm-core-parseJSON.h" // Include the new header


//#include <windows.h>
char prnBuf[1024];
char token[256];

Alarm alarm;

void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}

#define inFilename "alarm-config.json"

int main() {
    // Print a welcome message
    std::cout << "Windows Console Application: ALARM JSON Example\n";

    alarm.setDebugCallback(GlobalDebugLogger);
    alarm.debugCallback(LOG_ERR_OK, "test\n");

#ifndef ARDUINO
    std::ifstream inputFile(inFilename);
    if (!inputFile.is_open()) {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Error: Failed to open %s\n", inFilename);
        return false;
    }

    // Read the file into a string
    std::string jsonString((std::istreambuf_iterator<char>(inputFile)),
        std::istreambuf_iterator<char>());
    inputFile.close();
#else
    //readFile(LittleFS, "/foo.txt");
#endif

    // Parse JSON configuration and populate the alarm system
    if (parseJsonConfig(jsonString, alarm)) {
        std::cout << "Alarm Data Loaded from JSON successfully\n";
    }
    else {
        std::cerr << "Failed to load alarm configuration\n";
        return 1;
    }

    if(subscribeMQTTtopics())
		std::cout << "MQTT topics subscribed successfully\n";
	else {
		std::cerr << "Failed to subscribe to MQTT topics\n";
		return 1;
	}   

    runMQTTTests();


    // Print the zone configuration regardless of warnings
    //alarm.printConfigData(zoneTags, ZONE_TAGS_CNT, (byte*)&zone, PRTCLASS_ALL);
    //alarm.printAlarmOpts((byte*)&alarmGlobalOpts);

    //for (auto it = alarm.beginValidZones(); it != alarm.endValidZones(); ++it) {
    //    std::cout << it->zoneName << std::endl; // Access zoneName of valid zones
    //}
    //for (const auto& partition : alarm) {
    //    std::cout << partition.partitionName << std::endl;
    //}
    //alarm.printAlarmPartCfg();
    //alarm.printAlarmZones(0, MAX_ALARM_ZONES);
    //alarm.printAlarmPgms();
    return 0;
}
//
// wrapers for the Alarm class methods
//
int getZoneCount() {
    return alarm.getZoneCount();
}
//
int getZoneIndex(const char* name) {
    return alarm.getZoneIndex(name);
}
//
const char* getZoneName(int zoneIdx) {
    return alarm.getZoneName(zoneIdx);
}
//
//bool isZoneValid(int zoneIdx) {
//    return alarm.isZoneValid(zoneIdx);
//}
//
int getPartitionCount() {
    return alarm.getPartitionCount();
}
//
int getPartitionIndex(const char* name) {
    return alarm.getPartitionIndex(name);
}
//
const char* getPartitionName(int partIdx) {
    return alarm.getPartitionName(partIdx);
}
//
bool isPartitionValid(int partIdx) {
    return alarm.isPartitionValid(partIdx);
}
//
void setPartitionTarget(int partIdx, int action) {
    alarm.setPartitionTarget(partIdx, static_cast<ARM_METHODS_t>(action));
}
//
int getPgmCount() {
    return alarm.getPgmCount();
}
//
//bool isPgmValid(int pgmIdx) {
//    return alarm.isPgmValid(pgmIdx);
//}
//
int getPgmIndex(const char* name) {
    return alarm.getPgmIndex(name);
}
//
const char* getPgmName(int pgmIdx) {
    return alarm.getPgmName(pgmIdx);
}
//
bool setGlobalOptions(const char* opt_name, const char* opt_val) {
    return alarm.setGlobalOptions(opt_name, opt_val);
}
//
int getGlobalOptionsCnt() {
    return alarm.getGlobalOptionsCnt();
}
//
const char* getGlobalOptionKeyStr(int idx) {
    return alarm.getGlobalOptionKeyStr(idx);
}

void zoneCmd(void* param1, void* param2, void* param3) {
	return alarm.modifyZn(param1, param2, param3);
}

void pgmCmd(void* param1, void* param2, void* param3) {
	return alarm.modifyPgm(param1, param2, param3);
}
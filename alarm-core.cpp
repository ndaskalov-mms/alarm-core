#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include <windows.h>

#include <cstdarg>
#include <cstdio>

#include "debug.h"


#include "AlarmClass.h" 
#include "parseJSON.h" // Include the new header

unsigned long millis() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    //printf("The system time is: %02d:%02d:%02d\n", st.wHour, st.wMinute,st.wSecond);
    unsigned long res = (st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds);
    //printf("Time in mS: %lu\n", res);
    return res;
}

char prnBuf[1024];
char token[256];

Alarm alarm;

//void debugPrinter(const char* message, size_t length) {
//    printf("[DEBUG] %.*s\n", (int)length, message);
//}

int main() {
    // Print a welcome message
    std::cout << "Windows Console Application: ALARM JSON Example\n";

    alarm.setDebugCallback(GlobalDebugLogger);
    alarm.debugCallback(LOG_ERR_OK, "test\n");

    // Parse JSON configuration and populate the alarm system
    if (parseJsonConfig("alarm-config.json", alarm)) {
        std::cout << "Alarm Data Loaded from JSON successfully\n";
    }
    else {
        std::cerr << "Failed to load alarm configuration\n";
        return 1;
    }

    // Print the zone configuration regardless of warnings
    //alarm.printConfigData(zoneTags, ZONE_TAGS_CNT, (byte*)&zone, PRTCLASS_ALL);
    //alarm.printAlarmOpts((byte*)&alarmGlobalOpts);
    alarm.printAlarmPartCfg();
    alarm.printAlarmZones(0, MAX_ALARM_ZONES);
    alarm.printAlarmPgms();
    return 0;
}

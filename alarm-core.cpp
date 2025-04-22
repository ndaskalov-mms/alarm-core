#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"
#include <windows.h>

#include <cstdarg>
#include <cstdio>

#include "debug.h"

// Global debug logger function - can be used from any file
int GlobalDebugLogger(int level, const char* format, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    const char* levelStr = "UNKNOWN";
    switch (level) {
    case LOG_ERR_OK:        levelStr = "OK      "; break;
    case LOG_ERR_DEBUG:     levelStr = "DEBUG   "; break;
    case LOG_ERR_INFO:      levelStr = "INFO    "; break;
    case LOG_ERR_WARNING:   levelStr = "WARNING "; break;
    case LOG_ERR_CRITICAL:  levelStr = "CRITICAL"; break;
    default:            levelStr = "UNKNOWN "; break;
    }

    // Output to console - could be redirected to other streams as needed
    printf("[%s] %s", levelStr, buffer);

    return level;
}


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

void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}

int main() {
    // Print a welcome message
    std::cout << "Windows Console Application: ALARM JSON Example\n";

    alarm.setDebugCallback(debugPrinter);

    alarm.ErrWrite(ERR_CRITICAL, "Debug print %d\n", 1);

    // Parse JSON configuration and populate the alarm system
    if (parseJsonConfig("alarm-config.json", alarm)) {
        std::cout << "Alarm Data Loaded from JSON successfully\n";
    }
    else {
        std::cerr << "Failed to load alarm configuration\n";
        return 1;
    }

    return 0;
}

// debug.h
#pragma once

// Global debug logger accessible from anywhere
extern int GlobalDebugLogger(int level, const char* format, ...);

// Error level definitions (if not already defined elsewhere)
#ifndef LOG_ERR_OK
#define LOG_ERR_OK          0
#define LOG_ERR_INFO        2
#define LOG_ERR_DEBUG      -2
#define LOG_ERR_WARNING    -3
#define LOG_ERR_CRITICAL   -1
#endif



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

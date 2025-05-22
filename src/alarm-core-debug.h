// alarm-core-debug.h
#pragma once
#include <stdarg.h>

// Global debug logger accessible from anywhere
//extern int GlobalDebugLogger(int level, const char* format, ...);

// Error level definitions (if not already defined elsewhere)
//#ifndef LOG_ERR_OK
enum LogLevel_t { 
    LOG_ERR_OK        = 0,
    LOG_ERR_INFO      = 2,
    LOG_ERR_DEBUG     =-2,
    LOG_ERR_WARNING   =-3,
    LOG_ERR_CRITICAL  =-1,
};
//#endif

// Default log level (can be changed at runtime)
static LogLevel_t g_logLevel = LOG_ERR_DEBUG;

// Set log level at runtime
inline void SetLogLevel(LogLevel_t level) { g_logLevel = level; }

#ifdef ARDUINO
#include <Arduino.h>
inline void GlobalDebugLogger(LogLevel_t level, const char* fmt, ...) {
    if (level < g_logLevel) return;
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.println(buf);
}
#else
#include <cstdio>
inline void GlobalDebugLogger(LogLevel_t level, const char* fmt, ...) {
    if (level < g_logLevel) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    printf("\n");
}
#endif

#define LOG_DEBUG(...)    GlobalDebugLogger(LOG_ERR_DEBUG, __VA_ARGS__)
#define LOG_INFO(...)     GlobalDebugLogger(LOG_ERR_INFO, __VA_ARGS__)
#define LOG_WARNING(...)  GlobalDebugLogger(LOG_ERR_WARNING, __VA_ARGS__)
#define LOG_ERROR(...)    GlobalDebugLogger(LOG_ERR_ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) GlobalDebugLogger(LOG_ERR_CRITICAL, __VA_ARGS__)

//#endif
//
//
//
//// Global debug logger function - can be used from any file
//int GlobalDebugLogger(int level, const char* format, ...) {
//    static char buffer[1024];
//    va_list args;
//    va_start(args, format);
//    vsnprintf(buffer, sizeof(buffer), format, args);
//    va_end(args);
//
//    const char* levelStr = "UNKNOWN";
//    switch (level) {
//    case LOG_ERR_OK:        levelStr = "OK      "; break;
//    case LOG_ERR_DEBUG:     levelStr = "DEBUG   "; break;
//    case LOG_ERR_INFO:      levelStr = "INFO    "; break;
//    case LOG_ERR_WARNING:   levelStr = "WARNING "; break;
//    case LOG_ERR_CRITICAL:  levelStr = "CRITICAL"; break;
//    default:            levelStr = "UNKNOWN "; break;
//    }
//
//    // Output to console - could be redirected to other streams as needed
//    printf("[%s] %s", levelStr, buffer);
//
//    return level;
//}
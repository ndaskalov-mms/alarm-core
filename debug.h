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

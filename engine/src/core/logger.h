#pragma once

#include "defines.h"
#include "asserts.h"
#include "platform/platform.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if DRELEASE == 1
    #define LOG_DEBUG_ENABLED 0
    #define LOG_TRACE_ENABLED 0
#endif

enum LogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};

b8 InitializeLogging();
void ShutdownLogging();

DAPI void LogOutput(LogLevel level, char* message, ...);

#define DFATAL(message, ...) LogOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef DERROR
    #define DERROR(message, ...) LogOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
    #define DWARN(message, ...) LogOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
    #define DWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
    #define DINFO(message, ...) LogOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
    #define DINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
    #define DDEBUG(message, ...) LogOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
    #define DDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
    #define DTRACE(message, ...) LogOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
    #define DTRACE(message, ...)
#endif
#include "logger.h"

//TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

struct LoggerSystemState{
    b8 initialized;
};

static LoggerSystemState* statePtr;

b8 InitializeLogging(u64* memoryRequirement, void* state){
    *memoryRequirement = sizeof(LoggerSystemState);
    if(state == 0){
        return true;
    }

    statePtr = (LoggerSystemState*)state;
    statePtr->initialized = true;

    DFATAL("A test message: %f", 3.14f);
    DERROR("A test message: %f", 3.14f);
    DWARN("A test message: %f", 3.14f);
    DINFO("A test message: %f", 3.14f);
    DDEBUG("A test message: %f", 3.14f);
    DTRACE("A test message: %f", 3.14f);

    //TODO: create log file
    return true;
}

void ShutdownLogging(void* state){
    statePtr = 0;
    //TODO: cleanup logging/write queued entries
}

void LogOutput(LogLevel level, char* message, ...){
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 isError = level < LOG_LEVEL_WARN;

    //Need to move away from strict char limit at some point
    i32 msgLength = 32000;
    char outMessage[msgLength];
    memset(outMessage, 0, sizeof(outMessage));

    //this va list type workaround is because MSFT headers override the Clang va_list type with char* sometimes
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, msgLength, message, argPtr);
    va_end(argPtr);

    char outMessage2[msgLength];
    sprintf(outMessage2, "%s%s\n", levelStrings[level], outMessage);

    if(isError){
        PlatformConsoleWriteError(outMessage2, level);
    } else{
        PlatformConsoleWrite(outMessage2, level);
    }
}

void ReportAssertionFailure(char* expression, char* message, char* file, i32 line){
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '$s', in file: %s, line: %d\n", expression, message, file, line);
}
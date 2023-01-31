#include "logger.h"

//TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


void ReportAssertionFailure(char* expression, char* message, char* file, i32 line){
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '$s', in file: %s, line: %d\n", expression, message, file, line);
}

b8 InitializeLogging(){
    //TODO: create log file
    return TRUE;
}

void ShutdownLogging(){
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
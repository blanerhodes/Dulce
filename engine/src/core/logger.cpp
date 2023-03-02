#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"
#include "platform/filesystem.h"
#include "core/dstring.h"
#include "core/dmemory.h"

#include <stdarg.h>

struct LoggerSystemState{
    FileHandle log_file_handle;
};

static LoggerSystemState* logger_state_ptr;

void AppendToLogFile(char* message) {
    if (logger_state_ptr && logger_state_ptr->log_file_handle.is_valid) {
        u64 length = StringLength(message);
        u64 written = 0;
        if (!FileSystemWrite(&logger_state_ptr->log_file_handle, length, message, &written)) {
            PlatformConsoleWriteError("ERROR writing to console.log.", LOG_LEVEL_ERROR);
        }
    }
}

b8 InitializeLogging(u64* memoryRequirement, void* state){
    *memoryRequirement = sizeof(LoggerSystemState);
    if(state == 0){
        return true;
    }
    logger_state_ptr = (LoggerSystemState*)state;

    if (!FileSystemOpen("console.log", FILE_MODE_WRITE, false, &logger_state_ptr->log_file_handle)) {
        PlatformConsoleWriteError("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    DFATAL("A test message: %f", 3.14f);
    DERROR("A test message: %f", 3.14f);
    DWARN("A test message: %f", 3.14f);
    DINFO("A test message: %f", 3.14f);
    DDEBUG("A test message: %f", 3.14f);
    DTRACE("A test message: %f", 3.14f);

    return true;
}

void ShutdownLogging(void* state) {
    logger_state_ptr = 0;
    //TODO: cleanup logging/write queued entries
}

void LogOutput(LogLevel level, char* message, ...) {
    //TODO: all this stuff has to go on another thread since it's so slow
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < LOG_LEVEL_WARN;

    char out_message[32000];
    DZeroMemory(out_message, sizeof(out_message));

    //this va list type workaround is because MSFT headers override the Clang va_list type with char* sometimes
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    StringFormatV(out_message, message, arg_ptr);
    va_end(arg_ptr);

    StringFormat(out_message, "%s%s\n", level_strings[level], out_message);

    if(is_error){
        PlatformConsoleWriteError(out_message, level);
    } else{
        PlatformConsoleWrite(out_message, level);
    }

    AppendToLogFile(out_message);
}

void ReportAssertionFailure(char* expression, char* message, char* file, i32 line){
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '$s', in file: %s, line: %d\n", expression, message, file, line);
}
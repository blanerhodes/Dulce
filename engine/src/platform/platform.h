#pragma once

#include "defines.h"

struct PlatformState{
    void* internalState;
};

b8 PlatformStartup(PlatformState* platState, char* appName, i32 x, i32 y, i32 width, i32 height);

void PlatformShutdown(PlatformState* platState);

b8 PlatformPumpMessages(PlatformState* platState);

void* PlatformAllocate(u64 size, b8 aligned);
void PlatformFree(void* block, b8 aligned);
void* PlatformZeroMemory(void* block, u64 size);
void* PlatformCopyMemory(void* dest, void* source, u64 size);
void* PlatformSetMemory(void* dest, i32 value, u64 size);

void PlatformConsoleWrite(char* message, u8 color);
void PlatformConsoleWriteError(char* message, u8 color);

f64 PlatformGetAbsoluteTime();

void PlatformSleep(u64 ms);
#pragma once

#include "defines.h"

DAPI u64 StringLength(char* str);

DAPI char* StringDuplicate(char* str);

//Case sensitive
DAPI b8 StringsEqual(char* str0, char* str1);

DAPI i32 StringFormat(char* dest, char* format, ...);

DAPI i32 StringFormatV(char* dest, char* format, __builtin_va_list va_listp);
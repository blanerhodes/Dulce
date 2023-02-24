#include "core/dstring.h"
#include "core/dmemory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

u64 StringLength(char* str){
    return strlen(str);
}

char* StringDuplicate(char* str){
    u64 length = StringLength(str);
    char* copy = (char*)DAllocate(length + 1, MEMORY_TAG_STRING);
    DCopyMemory(copy, str, length + 1);
    return copy;
}

b8 StringsEqual(char* str0, char* str1){
    return strcmp(str0, str1) == 0;
}

i32 StringFormat(char* dest, char* format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = StringFormatV(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 StringFormatV(char* dest, char* format, __builtin_va_list va_listp) {
    if (dest) {
        // Big, but can fit on the stack.
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, va_listp);
        buffer[written] = 0;
        DCopyMemory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}
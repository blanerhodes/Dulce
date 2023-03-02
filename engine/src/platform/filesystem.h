#pragma once

#include "defines.h"

struct FileHandle{
    void* handle;
    b8 is_valid;
};

enum FileModes{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
};

DAPI b8 FileSystemExists(char* path);

DAPI b8 FileSystemOpen(char* path, FileModes mode, b8 binary, FileHandle* out_handle);

DAPI b8 FileSystemClose(FileHandle* out_handle);

//Reads up to a newline or EOF. Allocates *line_buf which must be free by caller
DAPI b8 FileSystemReadLine(FileHandle* handle, char** line_buf);

//Writes to provided file appending '\n' at the end
DAPI b8 FileSystemWriteLine(FileHandle* handle, char* text);

//Reads up to data_size bytes of data into out_bytes_read
//Allocates *out_data which must be freed by caller
DAPI b8 FileSystemRead(FileHandle* handle, u64 data_size, void* out_data, u64* out_bytes_read);

//Allocates *out_bytes which must be freed by caller
DAPI b8 FileSystemReadAllBytes(FileHandle* handle, u8** out_bytes, u64* out_bytes_read);

DAPI b8 FileSystemWrite(FileHandle* handle, u64 data_size, void* data, u64* out_bytes_written);
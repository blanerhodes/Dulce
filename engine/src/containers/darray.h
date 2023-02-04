#pragma once

#include "defines.h"

/*
Memory layout
u64 capacity = num elements that can be held
u64 length = num elements currently contained
u64 stride = size of each element in bytes
void* elements
*/

enum{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

DAPI void* _DarrayCreate(u64 length, u64 stride);
DAPI void _DarrayDestroy(void* array);
DAPI u64 _DarrayGetField(void* array, u64 field);
DAPI void _DarraySetField(void* array, u64 field, u64 value);
DAPI void* _DarrayResize(void* array);
DAPI void* _DarrayPush(void* array, void* valuePtr);
DAPI void _DarrayPop(void* array, void* dest);
DAPI void* _DarrayPopAt(void* array, u64 index, void* dest);
DAPI void* _DarrayInsertAt(void* array, u64 index, void* valuePtr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define DarrayCreate(type) _DarrayCreate(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define DarrayReserve(type) _DarrayCreate(capacity, sizeof(type))

#define DarrayDestroy(array) _DarrayDestroy(array);

#define DarrayPush(array, value) \
    { \
        decltype(value) temp = value; \
        array = (decltype(value)*)_DarrayPush(array, &temp); \
    } \

#define DarrayPop(array, valuePtr) _DarrayPop(array, valuePtr)

#define DarrayInsertAt(array, index, value) \
    { \
        decltype(value) temp = value; \
        array = _DarrayInsertAt(array, index, &temp); \
    } \

#define DarrayPopAt(array, index, valuePtr) _DarrayPopAt(array, index, valuePtr)

#define DarrayClear(array) _DarraySetField(array, DARRAY_LENGTH, 0)

#define DarrayCapacity(array) _DarrayGetField(array, DARRAY_CAPACITY)

#define DarrayLength(array) _DarrayGetField(array, DARRAY_LENGTH)

#define DarrayStride(array) _DarrayGetField(array, DARRAY_STRIDE)

#define DarraySetLength(array, value) _DarraySetField(array, DARRAY_LENGTH, value)
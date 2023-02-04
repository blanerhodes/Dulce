#include "darray.h"
#include "core/dmemory.h"
#include "core/logger.h"

void* _DarrayCreate(u64 length, u64 stride){
    u64 headerSize = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 arraySize = length * stride;
    u64* newArray = (u64*)DAllocate(headerSize + arraySize, MEMORY_TAG_DARRAY);
    DSetMemory(newArray, 0, headerSize + arraySize);
    newArray[DARRAY_CAPACITY] = length;
    newArray[DARRAY_LENGTH] = 0;
    newArray[DARRAY_STRIDE] = stride;
    return (void*)(newArray + DARRAY_FIELD_LENGTH);
}

void _DarrayDestroy(void* array){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 headerSize = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 totalSize = headerSize + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    DFree(header, totalSize, MEMORY_TAG_DARRAY);
}

u64 _DarrayGetField(void* array, u64 field){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _DarraySetField(void* array, u64 field, u64 value){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _DarrayResize(void* array){
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    void* temp = _DarrayCreate((DARRAY_RESIZE_FACTOR * DarrayCapacity(array)), stride);
    DCopyMemory(temp, array, length * stride);
    _DarraySetField(temp, DARRAY_LENGTH, length);
    _DarrayDestroy(array);
    return temp;
}

void* _DarrayPush(void* array, void* valuePtr){
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    if(length >= DarrayCapacity(array)){
        array = _DarrayResize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    DCopyMemory((void*)addr, valuePtr, stride);
    _DarraySetField(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _DarrayPop(void* array, void* dest){
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    u64 addr = (u64)array;
    addr -= ((length - 1) * stride);
    DCopyMemory(dest, (void*)addr, stride);
    _DarraySetField(array, DARRAY_LENGTH, length - 1);
}

void* _DarrayPopAt(void* array, u64 index, void* dest){
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    if(index >= length){
        DERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }
    u64 addr = (u64)array;
    DCopyMemory(dest, (void*)(addr + (index * stride)), stride);
    if(index != length -1){
        DCopyMemory((void*)(addr + (index * stride)), (void*)(addr + ((index + 1) * stride)), stride * (length-index));
    }
    _DarraySetField(array, DARRAY_LENGTH, length-1);
    return array;
}

void* _DarrayInsertAt(void* array, u64 index, void* valuePtr){
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    if(index >= length){
        DERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }
    if(length >= DarrayCapacity(array)){
        array = _DarrayResize(array);
    }
    u64 addr = (u64)array;
    if(index != length - 1){
        DCopyMemory((void*)(addr + ((index + 1) * stride)), (void*)(addr + (index * stride)), stride * (length-index));
    }
    DCopyMemory((void*)(addr + (index * stride)), valuePtr, stride);
    _DarraySetField(array, DARRAY_LENGTH, length + 1);
    return array;
}
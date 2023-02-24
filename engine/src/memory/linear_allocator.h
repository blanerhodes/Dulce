#pragma once

#include "defines.h"

struct LinearAllocator{
    u64 totalSize;
    u64 allocated;
    void* memory;
    b8 ownsMemory;
};

DAPI void AllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator);
DAPI void AllocatorDestroy(LinearAllocator* allocator);
DAPI void* AllocatorAllocate(LinearAllocator* allocator, u64 size);
DAPI void AllocatorFreeAll(LinearAllocator* allocator);
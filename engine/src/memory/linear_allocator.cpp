#include "linear_allocator.h"

#include "core/dmemory.h"
#include "core/logger.h"

void AllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator){
    if(outAllocator){
        outAllocator->totalSize = totalSize;
        outAllocator->allocated = 0;
        outAllocator->ownsMemory = memory == 0;
        if(memory){
            outAllocator->memory = memory;
        } else{
            outAllocator->memory = DAllocate(totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

void AllocatorDestroy(LinearAllocator* allocator){
    if(allocator){
        allocator->allocated = 0;
        if(allocator->ownsMemory && allocator->memory){
            DFree(allocator->memory, allocator->totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        } 
        allocator->memory = 0;
        allocator->totalSize = 0;
        allocator->ownsMemory = 0;
    }
}

void* AllocatorAllocate(LinearAllocator* allocator, u64 size){
    if(allocator && allocator->memory){
        if(allocator->allocated + size > allocator->totalSize){
            u64 remaining = allocator->totalSize - allocator->allocated;
            DERROR("Linear allocator allocate - Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return 0;
        }
        void* block = ((u8*)allocator->memory) + allocator->allocated;
        allocator->allocated += size;
        return block;
    }
    DERROR("Linear allocator allocate - provided allocator not initialized.");
    return 0;
}

void AllocatorFreeAll(LinearAllocator* allocator){
    if(allocator && allocator->memory){
        allocator->allocated = 0;
        DZeroMemory(allocator->memory, allocator->totalSize);
    }
}
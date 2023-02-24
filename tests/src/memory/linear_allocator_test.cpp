#include "linear_allocator_tests.h"
#include "../test_manager.h"
#include "../expect.h"

#include <defines.h>
#include <memory/linear_allocator.h>

u8 LinearAllocator_ShouldCreateAndDestroy(){
    LinearAllocator alloc = {};
    AllocatorCreate(sizeof(u64), 0, &alloc);
    ExpectIntNotEquals(0, alloc.memory);
    ExpectIntEquals(sizeof(u64), alloc.totalSize);
    ExpectIntEquals(0, alloc.allocated);

    AllocatorDestroy(&alloc);
    ExpectIntEquals(0, alloc.memory);
    ExpectIntEquals(0, alloc.totalSize);
    ExpectIntEquals(0, alloc.allocated);

    return true;
}

u8 LinearAllocator_SingleAllocateAllSpace(){
    u32 buffSize = 512 * sizeof(u64);
    LinearAllocator alloc = {};
    AllocatorCreate(buffSize, 0, &alloc);

    void* block = AllocatorAllocate(&alloc, buffSize);
    ExpectIntNotEquals(0, block);
    ExpectIntEquals(buffSize, alloc.allocated);

    AllocatorDestroy(&alloc);
    return true;
}

u8 LinearAllocator_MultiAllocateAllSpaceSuccess(){
    u64 maxAllocs = 1024;
    LinearAllocator alloc = {};
    AllocatorCreate(sizeof(u64) * maxAllocs, 0, &alloc);

    void* block = 0;
    for(u64 i = 0; i < maxAllocs; i++){
        block = AllocatorAllocate(&alloc, sizeof(u64));
        ExpectIntNotEquals(0, block);
        ExpectIntEquals(sizeof(u64) * (i + 1), alloc.allocated);
    }

    AllocatorDestroy(&alloc);
    return true;
}

u8 LinearAllocator_OverAllocateShouldError(){
    u64 maxAllocs = 3;
    LinearAllocator alloc = {};
    AllocatorCreate(sizeof(u64) * maxAllocs, 0, &alloc);

    void* block = 0;
    for(u64 i = 0; i < maxAllocs; i++){
        block = AllocatorAllocate(&alloc, sizeof(u64));
        ExpectIntNotEquals(0, block);
        ExpectIntEquals(sizeof(u64) * (i + 1), alloc.allocated);
    }

    DDEBUG("Note: The following error is intentionally caused by this test.");

    block = AllocatorAllocate(&alloc, sizeof(u64));
    ExpectIntEquals(0, block);
    ExpectIntEquals(sizeof(u64) * maxAllocs, alloc.allocated);

    AllocatorDestroy(&alloc);
    return true;
}

u8 LinearAllocator_AllocateAllSpaceThenFree(){
    u64 maxAllocs = 1024;
    LinearAllocator alloc = {};
    AllocatorCreate(sizeof(u64) * maxAllocs, 0, &alloc);

    void* block = 0;
    for(u64 i = 0; i < maxAllocs; i++){
        block = AllocatorAllocate(&alloc, sizeof(u64));
        ExpectIntNotEquals(0, block);
        ExpectIntEquals(sizeof(u64) * (i + 1), alloc.allocated);
    }

    AllocatorFreeAll(&alloc);
    ExpectIntEquals(0, alloc.allocated);

    AllocatorDestroy(&alloc);
    return true;

}

void LinearAllocatorRegisterTests(){
    RegisterTest(LinearAllocator_ShouldCreateAndDestroy, "LinearAllocator_ShouldCreateAndDestroy");
    RegisterTest(LinearAllocator_SingleAllocateAllSpace, "LinearAllocator_SingleAllocateAllSpace");
    RegisterTest(LinearAllocator_MultiAllocateAllSpaceSuccess, "LinearAllocator_MultiAllocateAllSpaceSuccess");
    RegisterTest(LinearAllocator_OverAllocateShouldError, "LinearAllocator_OverAllocateShouldError");
    RegisterTest(LinearAllocator_AllocateAllSpaceThenFree, "LinearAllocator_AllocateAllSpaceThenFree");
}
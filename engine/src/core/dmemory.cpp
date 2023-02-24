#include "dmemory.h"
#include "core/logger.h"
#include "platform/platform.h"
#include <string.h>
#include <stdio.h>

struct MemoryStats{
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "LINEAR_ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "
};

struct MemorySystemState{
    MemoryStats stats;
    u64 allocCount;
};

static MemorySystemState* statePtr;

void InitializeMemory(u64* memorySysRequirements, void* state){
    *memorySysRequirements = sizeof(MemorySystemState);
    if(state == 0){
        return;
    }
    statePtr = (MemorySystemState*)state;
    statePtr->allocCount = 0;

    PlatformZeroMemory(&statePtr->stats, sizeof(statePtr->stats));
}

void ShutdownMemory(void* state){
    statePtr = 0;
}

void* DAllocate(u64 size, MemoryTag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        DWARN("DAllocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    if(statePtr){
        statePtr->stats.totalAllocated += size;
        statePtr->stats.taggedAllocations[tag] += size;
        statePtr->allocCount++;
    }

    //TODO: memory alignment
    void* block = PlatformAllocate(size, false);
    PlatformZeroMemory(block, size);
    return block;
}

void DFree(void* block, u64 size, MemoryTag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        DWARN("DAllocate called using MEMORY_TAG_UNKNOWN. Re-class this deallocation.");
    }
    statePtr->stats.totalAllocated -= size;
    statePtr->stats.taggedAllocations[tag] -= size;
    
    //TODO: memory alignment
    PlatformFree(block, false);
}

void* DZeroMemory(void* block, u64 size){
    return PlatformZeroMemory(block, size);
}

void* DCopyMemory(void* dest, void* source, u64 size){
    return PlatformCopyMemory(dest, source, size);
}

void* DSetMemory(void* dest, i32 value, u64 size){
    return PlatformSetMemory(dest, value, size);
}

char* GetMemoryUsageStr(){
    u64 kib = 1024;
    u64 mib = 1024 * 1024;
    u64 gib = 1024 * 1024* 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for(u32 i = 0; i < MEMORY_TAG_MAX_TAGS; i++){
        char unit[] = "XiB";
        float amount = 1.0f;

        if(statePtr->stats.taggedAllocations[i] >= gib){
            unit[0] = 'G';
            amount = statePtr->stats.taggedAllocations[i] / (float)gib;
        } else if(statePtr->stats.taggedAllocations[i] >= mib){
            unit[0] = 'M';
            amount = statePtr->stats.taggedAllocations[i] / (float)mib;
        } else if(statePtr->stats.taggedAllocations[i] >= kib){
            unit[0] = 'K';
            amount = statePtr->stats.taggedAllocations[i] / (float)kib;
        } else{
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)statePtr->stats.taggedAllocations[i];
        } 

        i32 length = snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* outString = _strdup(buffer);
    return outString;
}

u64 GetMemoryAllocCount(){
    if(statePtr){
        return statePtr->allocCount;
    }
    return 0;
}
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

static MemoryStats stats;

void InitializeMemory(){
    PlatformZeroMemory(&stats, sizeof(stats));
}

void ShutdownMemory(){
}

void* DAllocate(u64 size, MemoryTag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        DWARN("DAllocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    stats.totalAllocated += size;
    stats.taggedAllocations[tag] += size;

    //TODO: memory alignment
    void* block = PlatformAllocate(size, FALSE);
    PlatformZeroMemory(block, size);
    return block;
}

void DFree(void* block, u64 size, MemoryTag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        DWARN("DAllocate called using MEMORY_TAG_UNKNOWN. Re-class this deallocation.");
    }
    stats.totalAllocated -= size;
    stats.taggedAllocations[tag] -= size;
    
    //TODO: memory alignment
    PlatformFree(block, FALSE);
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

        if(stats.taggedAllocations[i] >= gib){
            unit[0] = 'G';
            amount = stats.taggedAllocations[i] / (float)gib;
        } else if(stats.taggedAllocations[i] >= mib){
            unit[0] = 'M';
            amount = stats.taggedAllocations[i] / (float)mib;
        } else if(stats.taggedAllocations[i] >= kib){
            unit[0] = 'K';
            amount = stats.taggedAllocations[i] / (float)kib;
        } else{
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.taggedAllocations[i];
        } 

        i32 length = snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* outString = _strdup(buffer);
    return outString;
}
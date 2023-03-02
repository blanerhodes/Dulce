#include "dmemory.h"
#include "core/logger.h"
#include "platform/platform.h"
#include <string.h>
#include <stdio.h>

struct MemoryStats{
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
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
    u64 alloc_count;
};

static MemorySystemState* memory_state_ptr;

void MemorySystemInitialize(u64* memory_sys_requirements, void* state){
    *memory_sys_requirements = sizeof(MemorySystemState);
    if(state == 0){
        return;
    }
    memory_state_ptr = (MemorySystemState*)state;
    memory_state_ptr->alloc_count = 0;

    PlatformZeroMemory(&memory_state_ptr->stats, sizeof(memory_state_ptr->stats));
}

void MemorySystemShutdown(void* state){
    memory_state_ptr = 0;
}

void* DAllocate(u64 size, MemoryTag tag){
    if(tag == MEMORY_TAG_UNKNOWN){
        DWARN("DAllocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }
    if(memory_state_ptr){
        memory_state_ptr->stats.total_allocated += size;
        memory_state_ptr->stats.tagged_allocations[tag] += size;
        memory_state_ptr->alloc_count++;
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
    if(memory_state_ptr){
        memory_state_ptr->stats.total_allocated -= size;
        memory_state_ptr->stats.tagged_allocations[tag] -= size;
    }
    
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

        if(memory_state_ptr->stats.tagged_allocations[i] >= gib){
            unit[0] = 'G';
            amount = memory_state_ptr->stats.tagged_allocations[i] / (float)gib;
        } else if(memory_state_ptr->stats.tagged_allocations[i] >= mib){
            unit[0] = 'M';
            amount = memory_state_ptr->stats.tagged_allocations[i] / (float)mib;
        } else if(memory_state_ptr->stats.tagged_allocations[i] >= kib){
            unit[0] = 'K';
            amount = memory_state_ptr->stats.tagged_allocations[i] / (float)kib;
        } else{
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)memory_state_ptr->stats.tagged_allocations[i];
        } 

        i32 length = snprintf(buffer + offset, 8000, " %s: %.2f%s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }
    char* outString = _strdup(buffer);
    return outString;
}

u64 GetMemoryAllocCount(){
    if(memory_state_ptr){
        return memory_state_ptr->alloc_count;
    }
    return 0;
}
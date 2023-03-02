#pragma once

#include "defines.h"

enum MemoryTag{
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_MAX_TAGS
};

DAPI void MemorySystemInitialize(u64* memorySysRequirements, void* state);
DAPI void MemorySystemShutdown(void* state);

DAPI void* DAllocate(u64 size, MemoryTag tag);
DAPI void DFree(void* block, u64, MemoryTag tag);
DAPI void* DZeroMemory(void* block, u64 size);
DAPI void* DCopyMemory(void* dest, void* source, u64 size);
DAPI void* DSetMemory(void* dest, i32 value, u64 size);
DAPI char* GetMemoryUsageStr();
DAPI u64 GetMemoryAllocCount();
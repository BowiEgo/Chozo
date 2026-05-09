#include <Core/Header/Assert.h>
#include <Core/Memory/Memory.h>

#include <algorithm>
#include <cstring>
#include <mutex>
#include <new>

namespace CZ {

struct MemoryHeader {
    std::size_t size;
    MemoryUsage usage;
};

struct {
    MemoryProfile profile;
    const char* cstr;
    std::mutex mutex;
} sTable[]{
    { { MEMORY_USAGE_MISC }, "MEMORY_USAGE_MISC", {} },
    { { MEMORY_USAGE_RUNTIME }, "MEMORY_USAGE_RUNTIME", {} },
    { { MEMORY_USAGE_RENDER }, "MEMORY_USAGE_RENDER", {} },
    { { MEMORY_USAGE_SERIAL }, "MEMORY_USAGE_SERIAL", {} },
    { { MEMORY_USAGE_UI }, "MEMORY_USAGE_UI", {} },
    { { MEMORY_USAGE_JOB_SYSTEM }, "MEMORY_USAGE_JOB_SYSTEM", {} },
    { { MEMORY_USAGE_PHYSICS }, "MEMORY_USAGE_PHYSICS", {} },
    { { MEMORY_USAGE_ASSET }, "MEMORY_USAGE_ASSET", {} },
    { { MEMORY_USAGE_SCENE }, "MEMORY_USAGE_SCENE", {} },
    { { MEMORY_USAGE_SCHEMA }, "MEMORY_USAGE_SCHEMA", {} },
};

static_assert(sizeof(sTable) / sizeof(*sTable) == MEMORY_USAGE_LAST);

void* HeapMalloc(std::size_t size, MemoryUsage usage) {
    MemoryHeader* header = (MemoryHeader*)std::malloc(sizeof(MemoryHeader) + size);
    CZ_CORE_ASSERT(header);
    header->size  = size;
    header->usage = usage;
    {
        std::unique_lock<std::mutex> lock(sTable[usage].mutex);
        sTable[usage].profile.Current += size;
        sTable[usage].profile.Peak =
            std::max(sTable[usage].profile.Peak, sTable[usage].profile.Current);
    }
    return (void*)(header + 1);
}

void HeapFree(void* ptr) {
    MemoryHeader* header = (MemoryHeader*)ptr - 1;
    {
        std::unique_lock<std::mutex> lock(sTable[header->usage].mutex);
        sTable[header->usage].profile.Current -= header->size;
    }
    std::free((void*)header);
}

char* HeapStrdup(const char* cstr, MemoryUsage usage) {
    size_t len = strlen(cstr);
    char* str  = (char*)HeapMalloc(len + 1, usage);
    memcpy(str, cstr, len);
    str[len] = '\0';
    return str;
}

const MemoryProfile& GetMemoryProfile(MemoryUsage usage) { return sTable[(int)usage].profile; }

int GetMemoryLeaks(MemoryProfile* leaks) {
    int count = 0;
    for (int i = 0; i < (int)MEMORY_USAGE_LAST; i++) {
        if (sTable[i].profile.Current == 0) continue;
        if (leaks) leaks[count] = sTable[i].profile;
        count++;
    }
    return count;
}

const char* GetMemoryUsageCstr(MemoryUsage usage) { return sTable[(int)usage].cstr; }

} // namespace CZ
#include <Core/Header/Assert.hpp>
#include <Core/Memory/Memory.hpp>

#include <algorithm>
#include <cstring>
#include <mutex>
#include <new>

namespace CZ {

struct MemoryHeader {
    std::size_t size;
    MemoryUsage usage;
#ifdef CZ_DEBUG
    const char* file;
    uint32_t line;
#endif
};

struct AllocRecord {
    std::size_t size;
    MemoryUsage usage;
    const char* file;
    uint32_t line;
};
static std::unordered_map<void*, AllocRecord> g_AllocTracker;
static std::mutex g_AllocTrackerMutex;

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

static_assert(sizeof(sTable) / sizeof(*sTable) == s_MemoryUsageCount);

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

void* HeapMallocDebug(std::size_t size, MemoryUsage usage, const char* file, uint32_t line) {
    MemoryHeader* header = (MemoryHeader*)std::malloc(sizeof(MemoryHeader) + size);
    CZ_CORE_ASSERT(header);
    header->size  = size;
    header->usage = usage;
#ifdef CZ_DEBUG
    header->file = file;
    header->line = line;
#endif
    {
        std::unique_lock<std::mutex> lock(sTable[usage].mutex);
        sTable[usage].profile.Current += size;
        sTable[usage].profile.Peak =
            std::max(sTable[usage].profile.Peak, sTable[usage].profile.Current);
    }
    auto ptr = (void*)(header + 1);

#ifdef CZ_DEBUG
    {
        std::lock_guard lock(g_AllocTrackerMutex);
        g_AllocTracker[ptr] = { size, usage, file, line };
    }
#endif
    return ptr;
}

void HeapFree(void* ptr) {
    MemoryHeader* header = (MemoryHeader*)ptr - 1;
    {
        std::unique_lock<std::mutex> lock(sTable[header->usage].mutex);

        if (sTable[header->usage].profile.Current < header->size) {
#ifdef CZ_DEBUG
            CZ_CORE_LOG(Error,
                        "HeapFree underflow! usage: {0}, size: {1}, current: {2}. "
                        "Pointer: {3} (allocated at {4}:{5})",
                        GetMemoryUsageCstr(header->usage), header->size,
                        sTable[header->usage].profile.Current, (void*)ptr, header->file,
                        header->line);
#else
            CZ_CORE_LOG(Error,
                        "HeapFree underflow! usage: {0}, size: {1}, current: {2}. Pointer: {3}",
                        GetMemoryUsageCstr(header->usage), header->size,
                        sTable[header->usage].profile.Current, (void*)ptr);
#endif
            CZ_DEBUGBREAK();
            return;
        }

        sTable[header->usage].profile.Current -= header->size;
    }
#ifdef CZ_DEBUG
    {
        std::lock_guard lock(g_AllocTrackerMutex);
        g_AllocTracker.erase(ptr);
    }
#endif
    std::free((void*)header);
}

char* HeapStrdup(const char* cstr, MemoryUsage usage) {
    size_t len = strlen(cstr);
    char* str  = (char*)HEAP_MALLOC(len + 1, usage);
    memcpy(str, cstr, len);
    str[len] = '\0';
    return str;
}

const MemoryProfile& GetMemoryProfile(MemoryUsage usage) { return sTable[(int)usage].profile; }

int GetMemoryLeaks(MemoryProfile* leaks) {
    int count = 0;
    for (int i = 0; i < s_MemoryUsageCount; i++) {
        if (sTable[i].profile.Current == 0) continue;
        if (leaks) leaks[count] = sTable[i].profile;
        count++;
    }
    return count;
}

const char* GetMemoryUsageCstr(MemoryUsage usage) { return sTable[(int)usage].cstr; }

void ReportMemoryLeaks() {
#ifndef CZ_DEBUG
    MemoryProfile leaks[s_MemoryUsageCount];
    int leakCount = GetMemoryLeaks(leaks);

    if (leakCount == 0) {
        CZ_CORE_LOG(Info, "No memory leaks detected.");
        return;
    }

    CZ_CORE_LOG(Error, "Memory leaks detected! Total leaking categories: {}", leakCount);
    for (int i = 0; i < leakCount; ++i) {
        const auto& prof = leaks[i];
        CZ_CORE_LOG(Error, "  [{}] {} bytes (peak: {} bytes)", GetMemoryUsageCstr(prof.Usage),
                    prof.Current, prof.Peak);
    }

    CZ_DEBUGBREAK();
#else
    std::lock_guard lock(g_AllocTrackerMutex);
    if (g_AllocTracker.empty()) {
        CZ_CORE_LOG(Info, "No active allocations.");
        return;
    }
    CZ_CORE_LOG(Error, "Remaining allocations:");
    for (auto& [ptr, record] : g_AllocTracker) {
        CZ_CORE_LOG(Error, "  Ptr: {} | {} bytes | {} | {}:{}", ptr, record.size,
                    GetMemoryUsageCstr(record.usage), record.file, record.line);
    }
    CZ_DEBUGBREAK();
#endif
}

} // namespace CZ
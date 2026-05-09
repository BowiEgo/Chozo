#pragma once

#include <Core/Memory/MemoryTypes.h>

#include <cstddef>
#include <mutex>
#include <unordered_map>

namespace CZ {

class TypeMemoryTracker {
public:
    static TypeMemoryTracker& Get();

    void RecordAlloc(std::size_t typeId, std::size_t size);

    void RecordDealloc(std::size_t typeId, std::size_t size);

    MemoryProfile GetTypeProfile(std::size_t typeId) const;

    int GetTypeLeaks(MemoryProfile* outLeaks, int maxCount) const;

    void Reset();

private:
    struct PerTypeInfo {
        MemoryProfile Profile;
        mutable std::mutex Mtx;
    };

    std::unordered_map<std::size_t, PerTypeInfo> m_Map;
    std::mutex m_GlobalMtx;
};

template <typename T> inline std::size_t GetTypeId() {
    static const char id = 0;
    return reinterpret_cast<std::size_t>(&id);
}

} // namespace CZ
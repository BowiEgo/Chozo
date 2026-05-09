#include <Core/Memory/TypeMemoryTracker.h>

#include <cstddef>
#include <mutex>
#include <unordered_map>

namespace CZ {

TypeMemoryTracker& TypeMemoryTracker::Get() {
    static TypeMemoryTracker instance;
    return instance;
}

void TypeMemoryTracker::RecordAlloc(std::size_t typeId, std::size_t size) {
    std::lock_guard<std::mutex> lock(m_Map[typeId].Mtx);
    auto& prof = m_Map[typeId].Profile;
    prof.Current += size;
    prof.Peak = std::max(prof.Peak, prof.Current);
}

void TypeMemoryTracker::RecordDealloc(std::size_t typeId, std::size_t size) {
    std::lock_guard<std::mutex> lock(m_Map[typeId].Mtx);
    auto& prof = m_Map[typeId].Profile;
    if (prof.Current >= size) {
        prof.Current -= size;
    } else {
        prof.Current = 0;
    }
}

MemoryProfile TypeMemoryTracker::GetTypeProfile(std::size_t typeId) const {
    const auto it = m_Map.find(typeId);
    if (it != m_Map.end()) {
        std::lock_guard<std::mutex> lock(it->second.Mtx);
        return it->second.Profile;
    }
    return MemoryProfile{};
}

int TypeMemoryTracker::GetTypeLeaks(MemoryProfile* outLeaks, int maxCount) const {
    int count = 0;
    for (const auto& [typeId, info] : m_Map) {
        std::lock_guard<std::mutex> lock(info.Mtx);
        if (info.Profile.Current > 0) {
            if (outLeaks && count < maxCount) {
                outLeaks[count] = info.Profile;
            }
            ++count;
        }
    }
    return count;
}

void TypeMemoryTracker::Reset() {
    std::lock_guard<std::mutex> lock(m_GlobalMtx);
    m_Map.clear();
}

} // namespace CZ
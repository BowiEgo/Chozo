#include <mutex>

#include "CoreMinimal.h"

static std::unordered_set<void*> s_LiveReferences;
static std::mutex s_LiveReferenceMutex;

namespace RefUtils {

CORE_API void AddToLiveReferences(void* instance) {
    if (!instance) return;

    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    s_LiveReferences.insert(instance);
}

CORE_API void RemoveFromLiveReferences(void* instance) {
    if (!instance || s_LiveReferences.find(instance) == s_LiveReferences.end()) return;

    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    s_LiveReferences.erase(instance);
}

CORE_API bool IsLive(const void* instance) {
    if (!instance) return false;

    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    return s_LiveReferences.find(const_cast<void*>(instance)) != s_LiveReferences.end();
}
} // namespace RefUtils

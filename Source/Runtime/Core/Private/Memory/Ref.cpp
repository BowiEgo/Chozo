#include "CoreTypes.h"

#include <mutex>

namespace Chozo {

static std::unordered_set<void *> s_LiveReferences;
static std::mutex s_LiveReferenceMutex;

namespace RefUtils {

CORE_API void AddToLiveReferences(void *instance) {
    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    CZ_CORE_ASSERT(instance, "");
    s_LiveReferences.insert(instance);
}

CORE_API void RemoveFromLiveReferences(void *instance) {
    std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
    CZ_CORE_ASSERT(instance, "");
    CZ_CORE_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end(), "");
    s_LiveReferences.erase(instance);
}

CORE_API bool IsLive(void *instance) {
    CZ_CORE_ASSERT(instance, "");
    return s_LiveReferences.find(instance) != s_LiveReferences.end();
}
} // namespace RefUtils
} // namespace Chozo

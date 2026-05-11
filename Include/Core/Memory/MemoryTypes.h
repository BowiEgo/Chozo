#pragma once

#include <array>
#include <utility>

namespace CZ {

// enum MemoryUsage : uint32_t {
//     MEMORY_USAGE_MISC = 0,
//     MEMORY_USAGE_RUNTIME,
//     MEMORY_USAGE_RENDER,
//     MEMORY_USAGE_SERIAL,
//     MEMORY_USAGE_UI,
//     MEMORY_USAGE_JOB_SYSTEM,
//     MEMORY_USAGE_PHYSICS,
//     MEMORY_USAGE_ASSET,
//     MEMORY_USAGE_SCENE,
//     MEMORY_USAGE_SCHEMA,
//     MEMORY_USAGE_LAST,
// };

#define MEMORY_USAGE_LIST                                                                          \
    X(MEMORY_USAGE_MISC)                                                                           \
    X(MEMORY_USAGE_RUNTIME)                                                                        \
    X(MEMORY_USAGE_RENDER)                                                                         \
    X(MEMORY_USAGE_SERIAL)                                                                         \
    X(MEMORY_USAGE_UI)                                                                             \
    X(MEMORY_USAGE_JOB_SYSTEM)                                                                     \
    X(MEMORY_USAGE_PHYSICS)                                                                        \
    X(MEMORY_USAGE_ASSET)                                                                          \
    X(MEMORY_USAGE_SCENE)                                                                          \
    X(MEMORY_USAGE_SCHEMA)

#define X(name) name,
enum MemoryUsage : uint32_t { MEMORY_USAGE_LIST };
#undef X

#define X(name) +1
constexpr int s_MemoryUsageCount = MEMORY_USAGE_LIST;
#undef X

#define X(name) #name,
static constexpr std::array<const char*, s_MemoryUsageCount> MemoryUsageStrings = {
    MEMORY_USAGE_LIST
};
#undef X

struct MemoryProfile {
    MemoryUsage Usage   = MemoryUsage::MEMORY_USAGE_MISC;
    std::size_t Peak    = 0;
    std::size_t Current = 0;
};

} // namespace CZ
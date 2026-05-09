#pragma once

#include <utility>

namespace CZ {

enum MemoryUsage : uint32_t {
    MEMORY_USAGE_MISC = 0,
    MEMORY_USAGE_RUNTIME,
    MEMORY_USAGE_RENDER,
    MEMORY_USAGE_SERIAL,
    MEMORY_USAGE_UI,
    MEMORY_USAGE_JOB_SYSTEM,
    MEMORY_USAGE_PHYSICS,
    MEMORY_USAGE_ASSET,
    MEMORY_USAGE_SCENE,
    MEMORY_USAGE_SCHEMA,
    MEMORY_USAGE_LAST,
};

struct MemoryProfile {
    MemoryUsage Usage   = MemoryUsage::MEMORY_USAGE_MISC;
    std::size_t Peak    = 0;
    std::size_t Current = 0;
};

} // namespace CZ
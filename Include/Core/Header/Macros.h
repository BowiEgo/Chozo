#pragma once

#include <functional>
#include <string>

////////////////////////////////////////////////////////////////////////////
//============================ Debug =====================================//
////////////////////////////////////////////////////////////////////////////
#ifdef CZ_DEBUG
static constexpr bool GIsDebug = true;
#else
static constexpr bool GIsDebug = false;
#endif

// // External declaration to avoid "identifier not found" errors
// #ifdef CZ_DEBUG
// // [Note] Provide the missing implementation
// void CZ_DumpStackTrace() {
//     // TODO: Implement platform specific stack walking
//     // std::cout << "Stack trace dumping is not implemented yet." <<
//     std::endl;
// }
// #endif

// --- Platform Detection (Internal) ---
#if defined(_WIN32) || defined(_WIN64)
    #define CZ_INTERNAL_DEBUGBREAK() __debugbreak()
#elif defined(__linux__)
    #include <signal.h>
    #define CZ_INTERNAL_DEBUGBREAK() raise(SIGTRAP)
#elif defined(__APPLE__)
    #define CZ_INTERNAL_DEBUGBREAK() __builtin_trap()
#else
    #define CZ_INTERNAL_DEBUGBREAK()
#endif

// --- Public Debug Break Macro ---
// Only active in Debug builds to prevent accidental breaks in Release
#ifdef CZ_DEBUG
    // #define CZ_DEBUGBREAK() CZ_INTERNAL_DEBUGBREAK()
    #define CZ_DEBUGBREAK()
#else
    #define CZ_DEBUGBREAK()
#endif

////////////////////////////////////////////////////////////////////////////
//=========================== Global Macros ==============================//
////////////////////////////////////////////////////////////////////////////
// Use 1ULL to ensure we support up to 64-bit flags safely
#define BIT(x) (1ULL << (x))

// Support functions with any number of arguments
#define CZ_BIND_FN(fn) [this](auto&&... args) { return fn(std::forward<decltype(args)>(args)...); }

#define CZ_CONCAT_IMPL(a, b) a##b
#define CZ_CONCAT(a, b)      CZ_CONCAT_IMPL(a, b)

////////////////////////////////////////////////////////////////////////////
//================================ Hash ==================================//
////////////////////////////////////////////////////////////////////////////
inline void HashCombine(size_t& seed, size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
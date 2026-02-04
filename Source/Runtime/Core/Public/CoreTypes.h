#pragma once

#include <filesystem>
#include <functional>
#include <memory>

////////////////////////////////////////////////////////////////////////////
//========================= DLL Import/Export ============================//
////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#if defined(_WIN32)
    #define CHOZO_DLL_EXPORT __declspec(dllexport)
    #define CHOZO_DLL_IMPORT __declspec(dllimport)
#else
    #if defined(__GNUC__) || defined(__clang__)
        #define CHOZO_DLL_EXPORT __attribute__((visibility("default")))
        #define CHOZO_DLL_IMPORT __attribute__((visibility("default")))
    #else
        #define CHOZO_DLL_EXPORT
        #define CHOZO_DLL_IMPORT
    #endif
#endif

// ---XXX_API definitions---
// These macros are used for classes/functions in the XXX modules.
#ifdef CORE_EXPORTS
    #define CORE_API CHOZO_DLL_EXPORT
#else
    #define CORE_API CHOZO_DLL_IMPORT
#endif

#ifdef ENGINE_EXPORTS
    #define ENGINE_API CHOZO_DLL_EXPORT
#else
    #define ENGINE_API CHOZO_DLL_IMPORT
#endif

#ifdef RENDERCORE_EXPORTS
    #define RENDERCORE_API CHOZO_DLL_EXPORT
#else
    #define RENDERCORE_API CHOZO_DLL_IMPORT
#endif

////////////////////////////////////////////////////////////////////////////
//============================ Debug =====================================//
////////////////////////////////////////////////////////////////////////////

// External declaration to avoid "identifier not found" errors
extern void CZ_DumpStackTrace();

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
    #define CZ_DEBUGBREAK() CZ_INTERNAL_DEBUGBREAK()
#else
    #define CZ_DEBUGBREAK()
#endif

// --- Assertion System ---
#ifdef CZ_DEBUG
    // Internal assertion implementation
    #define CZ_INTERNAL_ASSERT_IMPL(type, condition, ...)                      \
        do {                                                                   \
            if (!(condition)) {                                                \
                CZ_##type##_LOG(                                               \
                    Fatal,                                                     \
                    "Assertion Failed: {0}\n\tat {1}:{2}\n\tMessage: {3}",     \
                    #condition, __FILE__, __LINE__, __VA_ARGS__);              \
                CZ_DumpStackTrace();                                           \
                CZ_DEBUGBREAK();                                               \
            }                                                                  \
        } while (0)

    #define CZ_CORE_ASSERT(condition, ...)                                     \
        CZ_INTERNAL_ASSERT_IMPL(CORE, condition, __VA_ARGS__)
    #define CZ_ASSERT(condition, ...)                                          \
        CZ_INTERNAL_ASSERT_IMPL(APP, condition, __VA_ARGS__)
#else
    // Stripped in Release builds
    #define CZ_CORE_ASSERT(condition, ...)
    #define CZ_ASSERT(condition, ...)
#endif

////////////////////////////////////////////////////////////////////////////
//=========================== Global Macros ==============================//
////////////////////////////////////////////////////////////////////////////

// Use 1ULL to ensure we support up to 64-bit flags safely
#define BIT(x) (1ULL << (x))

// Support functions with any number of arguments
#define CZ_BIND_EVENT_FN(fn)                                                   \
    [this](auto &&...args) { return fn(std::forward<decltype(args)>(args)...); }

////////////////////////////////////////////////////////////////////////////
//============================ Global Scope ==============================//
////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
using TCallback = std::function<T(Args &&...args)>;

////////////////////////////////////////////////////////////////////////////
//========================== Basic Type Aliases ==========================//
////////////////////////////////////////////////////////////////////////////

// Stick to explicit sizes for global basic types to avoid ambiguity
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using f32 = float;
using f64 = double;

using FByte = uint8;
using FUint = uint32;

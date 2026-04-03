#pragma once

#include "CoreAssert.h"
#include "PlatformTypes.h"

#include <functional>

#if defined(_WIN32)
    #define CZ_DLL_EXPORT __declspec(dllexport)
    #define CZ_DLL_IMPORT __declspec(dllimport)
#else
    #if defined(__GNUC__) || defined(__clang__)
        #define CZ_DLL_EXPORT __attribute__((visibility("default")))
        #define CZ_DLL_IMPORT __attribute__((visibility("default")))
    #else
        #define CZ_DLL_EXPORT
        #define CZ_DLL_IMPORT
    #endif
#endif

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
#define CZ_BIND_EVENT_FN(fn)                                                                       \
    [this](auto&&... args) { return fn(std::forward<decltype(args)>(args)...); }

////////////////////////////////////////////////////////////////////////////
//============================ Global Scope ==============================//
////////////////////////////////////////////////////////////////////////////
template <typename T, typename... Args> using TCallback = std::function<T(Args&&... args)>;
using CallbackHandle = uint32_t;

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

////////////////////////////////////////////////////////////////////////////
//================================ Enum ==================================//
////////////////////////////////////////////////////////////////////////////
/**
 * ENUM_CLASS_FLAGS - Macro to enable bitwise operations for enum classes.
 * Includes: |, &, ^, ~, |=, &=, ^=
 */
#define ENUM_CLASS_FLAGS(Enum)                                                                     \
    inline Enum operator|(Enum a, Enum b) {                                                        \
        return static_cast<Enum>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));             \
    }                                                                                              \
    inline Enum operator&(Enum a, Enum b) {                                                        \
        return static_cast<Enum>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));             \
    }                                                                                              \
    inline Enum operator^(Enum a, Enum b) {                                                        \
        return static_cast<Enum>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));             \
    }                                                                                              \
    inline Enum operator~(Enum a) { return static_cast<Enum>(~static_cast<uint32_t>(a)); }         \
    inline Enum& operator|=(Enum& a, Enum b) {                                                     \
        return reinterpret_cast<Enum&>(reinterpret_cast<uint32_t&>(a) |=                           \
                                       static_cast<uint32_t>(b));                                  \
    }                                                                                              \
    inline Enum& operator&=(Enum& a, Enum b) {                                                     \
        return reinterpret_cast<Enum&>(reinterpret_cast<uint32_t&>(a) &=                           \
                                       static_cast<uint32_t>(b));                                  \
    }                                                                                              \
    inline Enum& operator^=(Enum& a, Enum b) {                                                     \
        return reinterpret_cast<Enum&>(reinterpret_cast<uint32_t&>(a) ^=                           \
                                       static_cast<uint32_t>(b));                                  \
    }

/**
 * Checks if any of the specified flags are set in the given bitmask.
 * Works with any enum class that has an underlying uint32_t type.
 */
template <typename TEnum> [[nodiscard]] inline bool EnumHasAnyFlags(TEnum flags, TEnum contains) {
    // static_cast to the underlying type (usually uint32_t) for bitwise comparison
    using UnderlyingType = typename std::underlying_type<TEnum>::type;
    return (static_cast<UnderlyingType>(flags) & static_cast<UnderlyingType>(contains)) != 0;
}

/**
 * Checks if all of the specified flags are set in the given bitmask.
 */
template <typename TEnum> [[nodiscard]] inline bool EnumHasAllFlags(TEnum flags, TEnum contains) {
    using UnderlyingType = typename std::underlying_type<TEnum>::type;
    return (static_cast<UnderlyingType>(flags) & static_cast<UnderlyingType>(contains)) ==
           static_cast<UnderlyingType>(contains);
}

////////////////////////////////////////////////////////////////////////////
//================================ Hash ==================================//
////////////////////////////////////////////////////////////////////////////
inline void HashCombine(size_t& seed, size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
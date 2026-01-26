#pragma once

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

/**
 * XXX_API definitions
 * These macros are used for classes/functions in the XXX modules.
 */
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

#ifdef CZ_ENABLE_ASSERTS
// #ifdef ENGINE_PLATFORM_MACOS
// 	#define CZ_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Core, "Assertion Failed", ##__VA_ARGS__)
// 	#define CZ_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Client, "Assertion Failed", ##__VA_ARGS__)
// #else
// 	#define CZ_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Core, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
// 	#define CZ_ASSERT_MESSAGE_INTERNAL(...)  ::Chozo::Log::PrintAssertMessage(::Chozo::Log::Type::Client, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
// #endif

#define CZ_ASSERT(condition, ...)                                                                                                                                                                      \
    {                                                                                                                                                                                                  \
        if (!(condition)) {                                                                                                                                                                            \
            CZ_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                                                                                                            \
            CZ_DEBUG_BREAK;                                                                                                                                                                            \
        }                                                                                                                                                                                              \
    }
#define CZ_CORE_ASSERT(condition, ...)                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (!(condition)) {                                                                                                                                                                            \
            CZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                                                                                                       \
            CZ_DEBUG_BREAK;                                                                                                                                                                            \
        }                                                                                                                                                                                              \
    }
// #define CZ_CORE_ASSERT(condition, ...) { if(!(condition)) { CZ_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); CZ_DEBUG_BREAK; } }
// #define CZ_ASSERT(condition, ...) { if(!(condition)) { CZ_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); CZ_DEBUG_BREAK; } }
#else
#define CZ_ASSERT(condition, ...)
#define CZ_CORE_ASSERT(condition, ...)
#endif

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

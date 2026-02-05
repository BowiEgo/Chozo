#include "CoreTypes.h"

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
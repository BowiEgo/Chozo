#pragma once

#include <Core/Header/Macros.h>
#include <Core/Log/LogEnum.h>
#include <Core/Log/LogMacros.h>

#include <string>

using namespace CZ;

// --- Assertion System ---
static inline bool g_AssertThrowInsteadOfAbort = false;
static inline std::string g_LastAssertMessage;

#ifndef CZ_DEBUG
    // Internal assertion implementation
    #define CZ_INTERNAL_ASSERT_IMPL(type, condition, ...)                                          \
        do {                                                                                       \
            if (!(condition)) {                                                                    \
                std::string _msg = std::string(__VA_ARGS__);                                       \
                if (g_AssertThrowInsteadOfAbort) {                                                 \
                    g_LastAssertMessage = _msg;                                                    \
                    throw std::runtime_error(_msg);                                                \
                } else {                                                                           \
                    CZ_##type##_LOG(Fatal, "Assertion Failed: {0}\n\tat {1}:{2}\n\tMessage: {3}",  \
                                    #condition, __FILE__, __LINE__, _msg);                         \
                    CZ_DEBUGBREAK();                                                               \
                }                                                                                  \
            }                                                                                      \
        } while (0)

    #define CZ_CORE_ASSERT(condition, ...) CZ_INTERNAL_ASSERT_IMPL(CORE, condition, __VA_ARGS__)
    #define CZ_ASSERT(condition, ...)      CZ_INTERNAL_ASSERT_IMPL(APP, condition, __VA_ARGS__)
#else
    // Stripped in Release builds
    #define CZ_CORE_ASSERT(condition, ...)
    #define CZ_ASSERT(condition, ...)
#endif

#define CZ_ENABLE_ASSERT_THROW()     g_AssertThrowInsteadOfAbort = true
#define CZ_DISABLE_ASSERT_THROW()    g_AssertThrowInsteadOfAbort = false
#define CZ_GET_LAST_ASSERT_MESSAGE() g_LastAssertMessage
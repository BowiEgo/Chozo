#pragma once

#include <memory>
// #include "Ref.h"

#ifdef ENGINE_PLATFORM_WIN32
    #ifdef CHOZO_BUILD_DLL
        #define CHOZO_API __declspec(dllexport)
    #else
        #define CHOZO_API __declspec(dllimport)
    #endif
#else
    #define CHOZO_API
#endif

////////////////////////////////////////////////////////////////////////////
//============================ Debug =====================================//
////////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER)
    #define DEBUG_BREAK __debugbreak()
#elif defined(__APPLE__) || defined(__MACH__)
    #include <signal.h>
    #define DEBUG_BREAK __builtin_trap()
#elif defined(__linux__)
    #include <signal.h>
    #define DEBUG_BREAK raise(SIGTRAP)
#else
    #error "Platform not supported"
#endif

// #ifdef CZ_ENABLE_ASSERTS
    #define CZ_ASSERT(x, ...) { if(!(x)) { CZ_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK; } }
    #define CZ_CORE_ASSERT(x, ...) { if(!(x)) { CZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK; } }
// #else
//     #define CZ_ASSERT(x, ...)
//     #define CZ_CORE_ASSERT(x, ...)
// #endif

#define BIT(x) (1 << x)

#define CZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Chozo {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;
}
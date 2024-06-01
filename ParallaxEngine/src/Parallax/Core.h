#pragma once

#include <memory>

#ifdef _WIN64
    #ifdef PARALLAX_BUILD_DLL
        #define PARALLAX_API __declspec(dllexport)
    #else
        #define PARALLAX_API __declspec(dllimport)
    #endif
#else
    #define PARALLAX_API
#endif

////////////////////////////////////////////////////////////////////////////
/////////////////////////// Debug //////////////////////////////////////////
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

#ifdef PRX_ENABLE_ASSERTS
    #define PRX_ASSERT(x, ...) { if(!(x)) { PRX_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK; } }
    #define PRX_CORE_ASSERT(x, ...) { if(!(x)) { PRX_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); DEBUG_BREAK; } }
#else
    #define PRX_ASSERT(x, ...)
    #define PRX_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define PRX_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Parallax {

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;
}
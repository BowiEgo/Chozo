#pragma once

#ifdef _WIN64
    #ifdef PARALLAX_BUILD_DLL
        #define PARALLAX_API __declspec(dllexport)
    #else
        #define PARALLAX_API __declspec(dllimport)
    #endif
#else
    #define PARALLAX_API
#endif

#ifdef PRX_ENABLE_ASSERTS
    #define PRX_ASSERT(x, ...) { if(!(x)) { PRX_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define PRX_CORE_ASSERT(x, ...) { if(!(x)) { PRX_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define PRX_ASSERT(x, ...)
    #define PRX_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define PRX_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
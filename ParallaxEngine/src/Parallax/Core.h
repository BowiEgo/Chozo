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

#define BIT(x) (1 << x)
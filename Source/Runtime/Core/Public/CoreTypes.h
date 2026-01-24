#pragma once

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

using uint8 = std::uint8_t;
using int32 = std::int32_t;
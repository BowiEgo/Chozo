#pragma once

////////////////////////////////////////////////////////////////////////////
//============================ Platform ==================================//
////////////////////////////////////////////////////////////////////////////
// --- Platform Detection (Internal) ---
#if defined(_WIN32) || defined(_WIN64)
    #define CZ_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define CZ_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define CZ_PLATFORM_MACOS
    #include "Mac/MacFile.h"
    #include "Mac/MacUtils.h"
#endif

#pragma once

#if defined(CZ_PLATFORM_WINDOWS)
    #include "WindowsString.h"

#elif defined(CZ_PLATFORM_MACOS)
    #include "MacString.h"
#endif
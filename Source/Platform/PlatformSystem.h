#pragma once

#if defined(CZ_PLATFORM_WINDOWS)
    #include "WindowsSystem.h"

#elif defined(CZ_PLATFORM_MACOS)
    #include "MacSystem.h"
#endif
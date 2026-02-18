#pragma once

#if defined(CZ_PLATFORM_WINDOWS)
    #include "WindowsFile.h"

#elif defined(CZ_PLATFORM_MACOS)
    #include "MacFile.h"
#endif
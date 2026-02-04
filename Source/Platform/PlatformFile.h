#pragma once

#if defined(CHOZO_PLATFORM_WINDOWS)
    #include "WindowsFile.h"

#elif defined(CHOZO_PLATFORM_MACOS)
    #include "MacFile.h"
#endif
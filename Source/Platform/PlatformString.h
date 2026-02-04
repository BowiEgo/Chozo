#pragma once

#if defined(CHOZO_PLATFORM_WINDOWS)
    #include "WindowsString.h"

#elif defined(CHOZO_PLATFORM_MACOS)
    #include "MacString.h"
#endif
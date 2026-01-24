#pragma once

#include "CoreTypes.h"

namespace Chozo
{
    
    struct ENGINE_API FWindowDefinition {
        std::string Title;
        unsigned int Width;
        unsigned int Height;

        FWindowDefinition()
            : Title("Chozo Engine")
            , Width(1280)
            , Height(720)
        {}
    };
}

#pragma once

#include <stdint.h>

namespace Chozo {

    namespace Utils {

        template<typename T>
        static void SetFlag(uint32_t& flags, const bool condition, const T flag) {
            if (condition)
                flags |= flag;
            else
                flags &= ~flag;
        }
    }
}
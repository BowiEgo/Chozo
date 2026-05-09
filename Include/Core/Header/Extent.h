#pragma once

#include <Core/Header/Types.h>

struct Extent2D {
    uint32 Width;
    uint32 Height;

    bool operator==(const Extent2D& other) const {
        return Width == other.Width && Height == other.Height;
    }
};
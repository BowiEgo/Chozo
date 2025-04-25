#pragma once

namespace Chozo {

    enum class ColorSpace
    {
        None = 0,
        SRGB,
        LinearSRGB
    };

    namespace Color {


        inline float SRGBToLinear(const float c)
        {
            return (c < 0.04045f) ? c * 0.0773993808f : std::pow(c * 0.9478672986f + 0.0521327014f, 2.4f);
        }

        inline float LinearToSRGB(const float c)
        {
            return (c < 0.0031308) ? c * 12.92 : 1.055 * (std::pow(c, 0.41666)) - 0.055;
        }
    }
}
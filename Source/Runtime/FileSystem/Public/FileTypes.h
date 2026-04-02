#pragma once

// clang-format off
enum class EChannelLayout {
    Unknown,
    R, RG, RGB, RGBA,
    BGR, BGRA,
    // Depth/Stencil
    D, DS,
};

enum class EColorSpace {
    Unknown,
    Linear,
    sRGB,
};

enum class EPixelPrecision {
    Unknown,
    U8,      // 8-bit unsigned normalized
    U16,     // 16-bit unsigned normalized
    F16,     // 16-bit float
    F32,     // 32-bit float
    RGB9E5,
    R11G11B10F,
};
// clang-format on

struct FImageFormatDesc {
    EChannelLayout Layout = EChannelLayout::Unknown;
    EColorSpace Space = EColorSpace::Linear;
    EPixelPrecision Precision = EPixelPrecision::Unknown;

    bool IsValid() const {
        return Layout != EChannelLayout::Unknown && Space != EColorSpace::Unknown &&
               Precision != EPixelPrecision::Unknown;
    }
};

EChannelLayout LayoutFromChannels(int channels) {
    switch (channels) {
        case 1: return EChannelLayout::R;
        case 2: return EChannelLayout::RG;
        case 3: return EChannelLayout::RGB;
        case 4: return EChannelLayout::RGBA;
        default: return EChannelLayout::Unknown;
    }
}

int ChannelsFromLayout(EChannelLayout layout) {
    switch (layout) {
        case EChannelLayout::R: return 1;
        case EChannelLayout::RG: return 2;
        case EChannelLayout::RGB: return 3;
        case EChannelLayout::RGBA: return 4;
        case EChannelLayout::BGR: return 3;
        case EChannelLayout::BGRA: return 4;
        default: return 0;
    }
}

#pragma once

#include "FileTypes.h"
#include "RHITypes.h"

namespace ChozoUtils::FileSystem {

EPixelFormat PixelFormatFromDesc(const FImageFormatDesc& desc) {
    if (!desc.IsValid()) {
        CZ_LOG(LogTextureImporter, Warning, "Invalid image format description");
        return EPixelFormat::Unknown;
    }

    bool isSRGB  = (desc.Space == EColorSpace::sRGB);
    bool isBGR   = (desc.Layout == EChannelLayout::BGR || desc.Layout == EChannelLayout::BGRA);
    int channels = ChannelsFromLayout(desc.Layout);

    // --- 1. Special Packed Formats (R11G11B10F / RGB9E5) ---
    if (desc.Precision == EPixelPrecision::R11G11B10F) {
        return (channels == 3 && !isSRGB) ? EPixelFormat::R11G11B10F : EPixelFormat::Unknown;
    }
    if (desc.Precision == EPixelPrecision::RGB9E5) {
        return (channels == 3 && !isSRGB) ? EPixelFormat::RGB9E5 : EPixelFormat::Unknown;
    }

    // --- 2. Depth/Stencil Recognition ---
    if (desc.Layout == EChannelLayout::Depth) {
        if (desc.Precision == EPixelPrecision::U16) return EPixelFormat::D16_UNORM;
        if (desc.Precision == EPixelPrecision::F32) return EPixelFormat::D32F;
    }
    if (desc.Layout == EChannelLayout::DepthStencil) {
        return EPixelFormat::D24S8;
    }

    // --- 3. BGR Restriction Check ---
    if (isBGR && desc.Precision != EPixelPrecision::U8) {
        CZ_LOG(LogTextureImporter, Warning, "BGR layout only supported for U8 precision");
        return EPixelFormat::Unknown;
    }

    // --- 4. Main Precision Switch ---
    switch (desc.Precision) {
        case EPixelPrecision::U8:
            if (channels == 1) return EPixelFormat::R8_UNORM;
            if (channels == 2) return EPixelFormat::RG8_UNORM;
            if (channels == 3 || channels == 4) {
                return isBGR ? (isSRGB ? EPixelFormat::BGRA8_SRGB : EPixelFormat::BGRA8_UNORM)
                             : (isSRGB ? EPixelFormat::RGBA8_SRGB : EPixelFormat::RGBA8_UNORM);
            }
            break;

        case EPixelPrecision::U16:
            if (channels == 1) return EPixelFormat::R16_UNORM;
            if (channels == 2) return EPixelFormat::RG16_UNORM;
            if (channels == 3 || channels == 4) return EPixelFormat::RGBA16_UNORM;
            break;

        case EPixelPrecision::F16:
            if (channels == 1) return EPixelFormat::R16F;
            if (channels == 2) return EPixelFormat::RG16F;
            if (channels == 3 || channels == 4) return EPixelFormat::RGBA16F;
            break;

        case EPixelPrecision::F32:
            if (channels == 1) return EPixelFormat::R32F;
            if (channels == 2) return EPixelFormat::RG32F;
            if (channels == 3 || channels == 4) return EPixelFormat::RGBA32F;
            break;

        default: break;
    }

    return EPixelFormat::Unknown;
}
} // namespace ChozoUtils::FileSystem

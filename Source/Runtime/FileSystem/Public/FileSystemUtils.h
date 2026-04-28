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

    // Special handling for packed formats that require RGB layout and linear space
    if (desc.Precision == EPixelPrecision::R11G11B10F) {
        if (channels == 3 && desc.Space == EColorSpace::Linear) return EPixelFormat::R11G11B10F;
        CZ_LOG(LogTextureImporter, Warning,
               "R11G11B10F requires 3 channels and linear space, got %d channels, space=%d",
               channels, static_cast<int>(desc.Space));
        return EPixelFormat::Unknown;
    }
    if (desc.Precision == EPixelPrecision::RGB9E5) {
        if (channels == 3 && desc.Space == EColorSpace::Linear) return EPixelFormat::RGB9E5;
        CZ_LOG(LogTextureImporter, Warning,
               "RGB9E5 requires 3 channels and linear space, got %d channels, space=%d", channels,
               static_cast<int>(desc.Space));
        return EPixelFormat::Unknown;
    }

    if (isBGR &&
        (desc.Precision == EPixelPrecision::U16 || desc.Precision == EPixelPrecision::F16 ||
         desc.Precision == EPixelPrecision::F32)) {
        CZ_LOG(
            LogTextureImporter, Warning,
            "BGR layout with high bit depth (U16/F16/F32) is not supported, converting to Unknown");
        return EPixelFormat::Unknown;
    }

    switch (desc.Precision) {
        case EPixelPrecision::U8:
            if (channels == 1) return EPixelFormat::R8_UNORM;
            if (channels == 2) return EPixelFormat::RG8_UNORM;
            if (channels == 3 || channels == 4) {
                // Note: 3-channel formats are promoted to 4 channels with alpha filled as 255
                return isBGR ? (isSRGB ? EPixelFormat::BGRA8_SRGB : EPixelFormat::BGRA8_UNORM)
                             : (isSRGB ? EPixelFormat::RGBA8_SRGB : EPixelFormat::RGBA8_UNORM);
            }
            break;

        case EPixelPrecision::U16:
            if (channels == 1) return EPixelFormat::R16_UNORM;
            if (channels == 2) return EPixelFormat::RG16_UNORM;
            if (channels == 3 || channels == 4) {
                return EPixelFormat::RGBA16_UNORM; // Alpha 通道填充为 65535 (1.0)
            }
            break;

        case EPixelPrecision::F16:
            if (channels == 1) return EPixelFormat::R16F;
            if (channels == 2) return EPixelFormat::RG16F;
            if (channels == 3 || channels == 4) {
                return EPixelFormat::RGBA16F; // Alpha 通道填充为 1.0
            }
            break;

        case EPixelPrecision::F32:
            if (channels == 1) return EPixelFormat::R32F;
            if (channels == 2) return EPixelFormat::RG32F;
            if (channels == 3 || channels == 4) {
                return EPixelFormat::RGBA32F; // Alpha 通道填充为 1.0
            }
            break;

        default:
            CZ_LOG(LogTextureImporter, Warning, "Unsupported pixel precision: %d",
                   static_cast<int>(desc.Precision));
            break;
    }

    return EPixelFormat::Unknown;
}
} // namespace ChozoUtils::FileSystem
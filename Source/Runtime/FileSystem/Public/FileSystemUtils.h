#pragma once

#include "FileTypes.h"
#include "RHITypes.h"

namespace ChozoUtils::FileSystem {
EPixelFormat PixelFormatFromDesc(const FImageFormatDesc& desc) {
    if (!desc.IsValid()) return EPixelFormat::Unknown;

    bool isSRGB = (desc.Space == EColorSpace::sRGB);
    bool isBGR = (desc.Layout == EChannelLayout::BGR || desc.Layout == EChannelLayout::BGRA);
    int channels = ChannelsFromLayout(desc.Layout);

    // Special handling for precision that doesn't require channel conversion
    if (desc.Precision == EPixelPrecision::R11G11B10F) {
        // Only supports RGB layout and must be linear space
        if (channels == 3 && desc.Space == EColorSpace::Linear)
            return EPixelFormat::R11G11B10F;
        else
            return EPixelFormat::Unknown;
    }
    if (desc.Precision == EPixelPrecision::RGB9E5) {
        if (channels == 3 && desc.Space == EColorSpace::Linear)
            return EPixelFormat::RGB9E5;
        else
            return EPixelFormat::Unknown;
    }

    // 根据精度和通道数进行映射
    switch (desc.Precision) {
        case EPixelPrecision::U8:
            if (channels == 1) return EPixelFormat::R8_UNORM;
            if (channels == 2) return EPixelFormat::RG8_UNORM;
            if (channels == 3) {
                // 硬件不支持 RGB8，升级为 RGBA8（Alpha 填充 255）
                return isBGR ? (isSRGB ? EPixelFormat::BGRA8_SRGB : EPixelFormat::BGRA8_UNORM)
                             : (isSRGB ? EPixelFormat::RGBA8_SRGB : EPixelFormat::RGBA8_UNORM);
            }
            if (channels == 4) {
                return isBGR ? (isSRGB ? EPixelFormat::BGRA8_SRGB : EPixelFormat::BGRA8_UNORM)
                             : (isSRGB ? EPixelFormat::RGBA8_SRGB : EPixelFormat::RGBA8_UNORM);
            }
            break;

        case EPixelPrecision::U16:
            if (channels == 1) return EPixelFormat::R16_UNORM;
            if (channels == 2) return EPixelFormat::RG16_UNORM;
            if (channels == 3) {
                // 升级为 RGBA16_UNORM，因为无原生 RGB16
                return EPixelFormat::RGBA16_UNORM;
            }
            if (channels == 4) return EPixelFormat::RGBA16_UNORM;
            break;

        case EPixelPrecision::F16:
            if (channels == 1) return EPixelFormat::R16F;
            if (channels == 2) return EPixelFormat::RG16F;
            if (channels == 3) {
                // 升级为 RGBA16F
                return EPixelFormat::RGBA16F;
            }
            if (channels == 4) return EPixelFormat::RGBA16F;
            break;

        case EPixelPrecision::F32:
            if (channels == 1) return EPixelFormat::R32F;
            if (channels == 2) return EPixelFormat::RG32F;
            if (channels == 3) {
                // 升级为 RGBA32F
                return EPixelFormat::RGBA32F;
            }
            if (channels == 4) return EPixelFormat::RGBA32F;
            break;

        default: break;
    }

    return EPixelFormat::Unknown;
}
} // namespace ChozoUtils::FileSystem
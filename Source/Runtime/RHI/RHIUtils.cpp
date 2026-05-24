#include <Runtime/RHI/RHIUtils.hpp>

namespace CZ::RHIUtils {

bool IsDepthFormat(PixelFormat format) {
    return format == PixelFormat::D32_SFLOAT || format == PixelFormat::D24_UNORM_S8_UINT ||
           format == PixelFormat::D16_UNORM;
}

ShaderDataType ToShaderDataFormat(PixelFormat format) {
    switch (format) {
        // Single-channel
        case PixelFormat::R8_UNORM:
        case PixelFormat::R16_UNORM:
        case PixelFormat::R16F:
        case PixelFormat::R32F:
        case PixelFormat::D16_UNORM:
        case PixelFormat::D24_UNORM_S8_UINT:
        case PixelFormat::D32_SFLOAT: return ShaderDataType::Float;

        // Dual-channel
        case PixelFormat::RG8_UNORM:
        case PixelFormat::RG16_UNORM:
        case PixelFormat::RG16F:
        case PixelFormat::RG32F: return ShaderDataType::Float2;

        case PixelFormat::RGB9E5:
        case PixelFormat::R11G11B10F: return ShaderDataType::Float3;

        // 4-channel RGBA/BGRA
        case PixelFormat::RGBA8_UNORM:
        case PixelFormat::RGBA8_SRGB:
        case PixelFormat::BGRA8_UNORM:
        case PixelFormat::BGRA8_SRGB:
        case PixelFormat::RGBA16_UNORM:
        case PixelFormat::RGBA16F:
        case PixelFormat::RGBA32F: return ShaderDataType::Float4;

        default: return ShaderDataType::None;
    }
}

} // namespace CZ::RHIUtils
#pragma once

namespace Chozo
{

    enum class ImageFormat
	{
		None = 0,
		RED8I,
		RED8UI,
		RED16I,
		RED16UI,
		RED32I,
		RED32UI,
		RED32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGB8,
		RGB16F,
		RGBA,
		RGBA16F,
		RGBA32F,
        // Color
        RGBA8,
        // ID
        RED_INTEGER,

		B10R11G11UF,

		SRGB,

		DEPTH32FSTENCIL8UINT,
		DEPTH32F,
        // Depth/Stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

    enum class ImageParameter
    {
        None = 0,
        LINEAR,
        NEAREST,
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
		LINEAR_MIPMAP_LINEAR,
    };

	namespace Utils {
		
		static uint32_t GetBytesPerPixel(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None:           return 0;
				case ImageFormat::RED8I:          return 1;
				case ImageFormat::RED8UI:         return 1;
				case ImageFormat::RED16I:         return 2;
				case ImageFormat::RED16UI:        return 2;
				case ImageFormat::RED32I:         return 4;
				case ImageFormat::RED32UI:        return 4;
				case ImageFormat::RED32F:         return 4;
				case ImageFormat::RG8:            return 2;
				case ImageFormat::RG16F:          return 4;  // 16-bit float, 2 components (RG) = 2 * 2
				case ImageFormat::RG32F:          return 8;  // 32-bit float, 2 components (RG) = 2 * 4
				case ImageFormat::RGB:            return 3;  // 8-bit per component, 3 components
				case ImageFormat::RGB8:           return 3;  // 8-bit per component, 3 components
				case ImageFormat::RGB16F:         return 6;  // 16-bit float, 3 components (RGB) = 3 * 2
				case ImageFormat::RGBA:           return 4;  // 8-bit per component, 4 components (RGBA)
				case ImageFormat::RGBA8:          return 4;  // 8-bit per component, 4 components (RGBA)
				case ImageFormat::RGBA16F:        return 8;  // 16-bit float, 4 components (RGBA) = 4 * 2
				case ImageFormat::RGBA32F:        return 16; // 32-bit float, 4 components (RGBA) = 4 * 4
				case ImageFormat::RED_INTEGER:    return 4;  // Assuming 32-bit integer
				case ImageFormat::B10R11G11UF:    return 4;  // 10-bit R, 11-bit G, 11-bit B packed into 4 bytes
				case ImageFormat::SRGB:           return 3;  // SRGB, typically 8-bit per channel, 3 components
				case ImageFormat::DEPTH32FSTENCIL8UINT: return 5; // 32-bit float depth + 8-bit stencil = 4 + 1 bytes
				case ImageFormat::DEPTH32F:       return 4;  // 32-bit float for depth
				case ImageFormat::DEPTH24STENCIL8:return 4;  // 24-bit depth + 8-bit stencil packed into 4 bytes
			}
			return 0;
		}
	} // namespace Utils
	
}

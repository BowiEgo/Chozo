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
}

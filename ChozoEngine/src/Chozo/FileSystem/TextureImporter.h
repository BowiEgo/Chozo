#pragma once

#include "Chozo/Renderer/Texture.h"

#include <filesystem>

namespace Chozo {

	class TextureImporter
	{
	public:
		static Buffer ToBufferFromFile(const std::string &path, ImageFormat& outFormat, uint32_t& outWidth, uint32_t& outHeight, bool flipY = false);
		static Buffer ToBufferFromMemory(Buffer buffer, ImageFormat& outFormat, uint32_t& outWidth, uint32_t& outHeight);
        static float ExtractGammaFromHDR(const std::string& filepath);
	};
}

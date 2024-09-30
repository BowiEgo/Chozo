#pragma once

#include "Chozo/Renderer/Texture.h"

#include <filesystem>

namespace Chozo {

	class TextureExporter
	{
	public:
		static Buffer ToFileFromBuffer(std::filesystem::path path, Buffer sourceBuffer, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetWidth, uint32_t targetHeight, bool isHDR = false);
    private:
        static void ResizeImageData(void* sourceData, void* targetData, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetWidth, uint32_t targetHeight);
        static void ResizeHDRImageData(void* sourceData, void* targetData, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetWidth, uint32_t targetHeight, int channels);
        static bool WriteToPNG(const std::string& path, void* data, uint32_t width, uint32_t height, int channels);
        static void ApplyGammaCorrectionAndClamp(float* sourceData, unsigned char* targetData, uint32_t size, float gamma);
	};
}

#include "TextureImporter.h"

#include "stb_image.h"

namespace Chozo {

    Buffer TextureImporter::ToBufferFromFile(const std::string &path, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight)
    {
		Buffer imageBuffer;

		int width, height, channels;
        stbi_set_flip_vertically_on_load(1);

        if (stbi_is_hdr(path.c_str()))
		{
			imageBuffer.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 4);
			imageBuffer.Size = width * height * 4 * sizeof(float);
			outFormat = ImageFormat::RGBA32F;
		}
        else
		{
			imageBuffer.Data = stbi_load(path.c_str(), &width, &height, &channels, 4);
			imageBuffer.Size = width * height * 4;
			outFormat = ImageFormat::RGBA;
		}

		if (!imageBuffer.Data)
        {
            CZ_CORE_ERROR("Failed to load image! {}" , stbi_failure_reason());
            return {};
        }

        outWidth = width;
		outHeight = height;
		return imageBuffer;
    }

    Buffer TextureImporter::ToBufferFromMemory(Buffer buffer, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight)
    {
        Buffer imageBuffer;

		int width, height, channels;
        stbi_set_flip_vertically_on_load(1);

		if (stbi_is_hdr_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size))
		{
			imageBuffer.Data = (byte*)stbi_loadf_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size, &width, &height, &channels, STBI_rgb_alpha);
			imageBuffer.Size = width * height * 4 * sizeof(float);
			outFormat = ImageFormat::RGBA32F;
		}
		else
		{
			imageBuffer.Data = stbi_load_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size, &width, &height, &channels, STBI_rgb_alpha);
			imageBuffer.Size = width * height * 4;
			outFormat = ImageFormat::RGBA;
		}

		if (!imageBuffer.Data)
        {
            CZ_CORE_ERROR("Failed to load image! {}" , stbi_failure_reason());
			return {};
        }

		outWidth = width;
		outHeight = height;
		return imageBuffer;
    }
}

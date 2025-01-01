#include "TextureImporter.h"

#include "stb_image.h"

namespace Chozo {

    Buffer TextureImporter::ToBufferFromFile(const std::string &path, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight, const bool flipY)
    {
		Buffer imageBuffer;

		int width, height, channels;
        stbi_set_flip_vertically_on_load(flipY);

        if (stbi_is_hdr(path.c_str()))
		{
            // float gamma = ExtractGammaFromHDR(path);
            // CZ_CORE_INFO("file: {}, gamma: {}", path, gamma);
			imageBuffer.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 4);
			imageBuffer.Size = width * height * 4 * sizeof(float);
			outFormat = ImageFormat::HDR;
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

    Buffer TextureImporter::ToBufferFromMemory(Buffer buffer, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight, const bool flipY)
    {
        Buffer imageBuffer;

		int width, height, channels;
        stbi_set_flip_vertically_on_load(flipY);

		if (stbi_is_hdr_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size))
		{
			imageBuffer.Data = (byte*)stbi_loadf_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size, &width, &height, &channels, STBI_rgb_alpha);
			imageBuffer.Size = width * height * 4 * sizeof(float);
			outFormat = ImageFormat::HDR;
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

    float TextureImporter::ExtractGammaFromHDR(const std::string& filepath)
    {
        std::ifstream file(filepath);
        std::string line;
        float gamma = 2.2f; // Default gamma value if none is found

        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << filepath << std::endl;
            return gamma;
        }

        // Read the file line by line
        while (std::getline(file, line))
        {
            // Stop reading the header when an empty line or resolution is encountered
            if (line.empty() || line[0] == '-' || line.find("FORMAT=") != std::string::npos)
                break;

            // Look for the line starting with "GAMMA="
            if (line.find("GAMMA=") != std::string::npos)
            {
                // Extract the gamma value
                std::string gammaValue = line.substr(6); // Skip "GAMMA="
                gamma = std::stof(gammaValue);
                break;
            }
        }

        file.close();
        return gamma;
    }
}

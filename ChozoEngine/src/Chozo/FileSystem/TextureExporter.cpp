#include "TextureExporter.h"

#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image_resize2.h"

namespace Chozo {

    Buffer TextureExporter::ToFileFromBuffer(fs::path path, Buffer sourceBuffer, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetWidth, uint32_t targetHeight, bool isHDR)
    {
        std::string filepath = path.string() + ".png";
        int channels = 4; // 4 channels for RGBA

        Buffer targetBuffer;
        uint64_t size = targetWidth * targetHeight * channels;
        targetBuffer.Allocate(size);

        if (isHDR)
            ResizeHDRImageData(sourceBuffer.Data, targetBuffer.Data, sourceWidth, sourceHeight, targetWidth, targetHeight, channels);
        else
            ResizeImageData(sourceBuffer.Data, targetBuffer.Data, sourceWidth, sourceHeight, targetWidth, targetHeight);

        bool success = WriteToPNG(filepath, targetBuffer.Data, targetWidth, targetHeight, channels);
        if (!success)
        {
            targetBuffer.Release();
            return Buffer();
        }

        return targetBuffer;
    }

    void TextureExporter::ResizeImageData(void* sourceData, void* targetData, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetWidth, uint32_t targetHeight)
    {
        stbir_resize_uint8_linear((unsigned char*)sourceData, sourceWidth, sourceHeight, 0,
                   (unsigned char*)targetData, targetWidth, targetHeight, 0,
                   stbir_pixel_layout::STBIR_RGBA);
    }

    void TextureExporter::ResizeHDRImageData(void* sourceData, void* targetData, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetWidth, uint32_t targetHeight, int channels)
    {
        // Create temporary buffer to hold the 8-bit RGBA data
        std::vector<unsigned char> hdrData(sourceWidth * sourceHeight * channels);

        // Apply gamma correction and convert float HDR data to 8-bit RGBA
        ApplyGammaCorrectionAndClamp((float*)sourceData, hdrData.data(), sourceWidth * sourceHeight * channels, 2.2f);
        ResizeImageData(hdrData.data(), targetData, sourceWidth, sourceHeight, targetWidth, targetHeight);
    }

    bool TextureExporter::WriteToPNG(const std::string& path, void* data, uint32_t width, uint32_t height, int channels)
    {
        stbi_flip_vertically_on_write(1); 
        bool success = stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
        if (!success)
        {
            CZ_CORE_ERROR("Failed to write image to file: {}", path);
            return false;
        }

        return true;
    }

    void TextureExporter::ApplyGammaCorrectionAndClamp(float* sourceData, unsigned char* targetData, uint32_t size, float gamma)
    {
        for (uint32_t i = 0; i < size; ++i)
        {
            // Crop to range [0.0, 1.0]
            float pixel = sourceData[i];
            if (pixel < 0.0f)
                pixel = 0.0f;
            if (pixel > 1.0f)
                pixel = 1.0f;

            // Gamma correction
            targetData[i] = static_cast<unsigned char>(255.0f * std::pow(pixel, 1.0f / 2.2f));
        }
    }
}

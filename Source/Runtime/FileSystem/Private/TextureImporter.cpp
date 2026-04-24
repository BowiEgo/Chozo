#include "TextureImporter.h"

#include "stb_image.h"

DEFINE_LOG_CATEGORY(LogTextureImporter);

FBuffer FTextureImporter::ToBufferFromFile(const std::string& path, FImageFormatDesc& outDesc,
                                           uint32_t& outWidth, uint32_t& outHeight,
                                           const bool flipY) {
    FBuffer imageBuffer;

    int w, h, channels;
    stbi_set_flip_vertically_on_load(flipY);

    if (!stbi_info(path.c_str(), &w, &h, &channels)) {
        return {};
    }

    int is16bit = stbi_is_16_bit(path.c_str());
    int isHDR   = stbi_is_hdr(path.c_str());

    CZ_LOG(LogTextureImporter, Trace,
           "Loading image from file: {}, 16-bit: {}, HDR: {}, width: {}, height: {}, channels: {}",
           path, is16bit ? "Yes" : "No", isHDR ? "Yes" : "No", w, h, channels);

    outDesc.Layout = LayoutFromChannels(channels);
    outDesc.Space  = (isHDR || is16bit)
                         ? EColorSpace::Linear
                         : EColorSpace::sRGB; // default to sRGB for LDR, linear for HDR/16-bit
    if (isHDR) {
        outDesc.Precision = EPixelPrecision::F32;
    } else if (is16bit) {
        outDesc.Precision = EPixelPrecision::U16;
    } else {
        outDesc.Precision = EPixelPrecision::U8;
    }

    if (isHDR) {
        // float gamma = ExtractGammaFromHDR(path);
        imageBuffer.Data = stbi_loadf(path.c_str(), &w, &h, &channels, 4);
        imageBuffer.Size = w * h * 4 * sizeof(float);
    } else if (is16bit) {
        imageBuffer.Data = stbi_load_16(path.c_str(), &w, &h, &channels, 0);
        imageBuffer.Size = w * h * channels * sizeof(uint16_t);
    } else {
        imageBuffer.Data = stbi_load(path.c_str(), &w, &h, &channels, 0);
        imageBuffer.Size = w * h * channels;
    }

    if (!imageBuffer.Data) {
        CZ_LOG(LogTextureImporter, Error, "Failed to load image from {}! {}", path,
               stbi_failure_reason());
        return {};
    }

    if (!isHDR && channels != 4) {
        int newChannels      = 4;
        size_t bytesPerPixel = is16bit ? sizeof(uint16_t) : 1;
        size_t newSize       = w * h * newChannels * bytesPerPixel;
        void* newData        = malloc(newSize);
        if (!newData) {
            CZ_LOG(LogTextureImporter, Error, "Out of memory while converting texture channels");
            free(imageBuffer.Data);
            return {};
        }

        const uint8_t* src8   = static_cast<const uint8_t*>(imageBuffer.Data);
        const uint16_t* src16 = static_cast<const uint16_t*>(imageBuffer.Data);
        uint8_t* dst8         = static_cast<uint8_t*>(newData);
        uint16_t* dst16       = static_cast<uint16_t*>(newData);
        bool isBGR =
            (outDesc.Layout == EChannelLayout::BGR || outDesc.Layout == EChannelLayout::BGRA);

        for (size_t i = 0; i < w * h; ++i) {
            uint16_t r = 0, g = 0, b = 0, a = (is16bit ? 65535 : 255);
            switch (channels) {
                case 1:
                    if (is16bit) {
                        r = g = b = *src16;
                        src16 += 1;
                    } else {
                        r = g = b = *src8;
                        src8 += 1;
                    }
                    break;
                case 2:
                    if (is16bit) {
                        r = g = b = src16[0];
                        a         = src16[1];
                        src16 += 2;
                    } else {
                        r = g = b = src8[0];
                        a         = src8[1];
                        src8 += 2;
                    }
                    break;
                case 3:
                    if (isBGR) {
                        if (is16bit) {
                            b = src16[0];
                            g = src16[1];
                            r = src16[2];
                            src16 += 3;
                        } else {
                            b = src8[0];
                            g = src8[1];
                            r = src8[2];
                            src8 += 3;
                        }
                    } else {
                        if (is16bit) {
                            r = src16[0];
                            g = src16[1];
                            b = src16[2];
                            src16 += 3;
                        } else {
                            r = src8[0];
                            g = src8[1];
                            b = src8[2];
                            src8 += 3;
                        }
                    }
                    break;
                default:
                    if (is16bit) {
                        r = src16[0];
                        g = src16[1];
                        b = src16[2];
                        a = src16[3];
                        src16 += 4;
                    } else {
                        r = src8[0];
                        g = src8[1];
                        b = src8[2];
                        a = src8[3];
                        src8 += 4;
                    }
                    break;
            }

            if (is16bit) {
                dst16[0] = r;
                dst16[1] = g;
                dst16[2] = b;
                dst16[3] = a;
                dst16 += 4;
            } else {
                dst8[0] = r;
                dst8[1] = g;
                dst8[2] = b;
                dst8[3] = a;
                dst8 += 4;
            }
        }

        free(imageBuffer.Data);
        imageBuffer.Data = newData;
        imageBuffer.Size = newSize;
        outDesc.Layout   = EChannelLayout::RGBA;
    }

    outWidth  = w;
    outHeight = h;
    return imageBuffer;
}

FBuffer FTextureImporter::ToBufferFromMemory(FBuffer buffer, FImageFormatDesc& outDesc,
                                             uint32_t& outWidth, uint32_t& outHeight,
                                             const bool flipY) {
    FBuffer imageBuffer;

    int w, h, channels;
    stbi_set_flip_vertically_on_load(flipY);

    if (stbi_is_hdr_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size)) {
        imageBuffer.Data = (FByte*)stbi_loadf_from_memory(
            (const stbi_uc*)buffer.Data, (int)buffer.Size, &w, &h, &channels, STBI_rgb_alpha);
        imageBuffer.Size = w * h * 4 * sizeof(float);
        // outFormat = EImageFormat::HDR;
    } else {
        imageBuffer.Data = stbi_load_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size, &w,
                                                 &h, &channels, STBI_rgb_alpha);
        imageBuffer.Size = w * h * 4;
        // outFormat = EImageFormat::RGBA;
    }

    if (!imageBuffer.Data) {
        CZ_LOG(LogTextureImporter, Error, "Failed to load image from buffer! {}",
               stbi_failure_reason());
        return {};
    }

    outWidth  = w;
    outHeight = h;
    return imageBuffer;
}

// float FTextureImporter::ExtractGammaFromHDR(const std::string& filepath) {
//     std::ifstream file(filepath);
//     std::string line;
//     float gamma = 2.2f; // Default gamma value if none is found

//     if (!file.is_open()) {
//         std::cerr << "Failed to open file: " << filepath << std::endl;
//         return gamma;
//     }

//     // Read the file line by line
//     while (std::getline(file, line)) {
//         // Stop reading the header when an empty line or resolution is encountered
//         if (line.empty() || line[0] == '-' || line.find("FORMAT=") != std::string::npos) break;

//         // Look for the line starting with "GAMMA="
//         if (line.find("GAMMA=") != std::string::npos) {
//             // Extract the gamma value
//             std::string gammaValue = line.substr(6); // Skip "GAMMA="
//             gamma = std::stof(gammaValue);
//             break;
//         }
//     }

//     file.close();
//     return gamma;
// }
#pragma once

#include "Buffer.h"
#include "FileTypes.h"

#include "FileSystemExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTextureImporter, Info);

class FILE_SYSTEM_API FTextureImporter {
public:
    static FBuffer ToBufferFromFile(const std::string& path, FImageFormatDesc& outDecs,
                                    uint32_t& outWidth, uint32_t& outHeight, bool flipY = true);
    static FBuffer ToBufferFromMemory(FBuffer buffer, FImageFormatDesc& outDecs, uint32_t& outWidth,
                                      uint32_t& outHeight, bool flipY = true);
    static float ExtractGammaFromHDR(const std::string& filepath);
};
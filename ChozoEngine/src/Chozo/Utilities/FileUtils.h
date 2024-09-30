#pragma once

#include "czpch.h"

namespace Chozo::Utils {

    namespace File {

        static const char* GetShaderCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "../caches/shader/opengl";
        }

        static const char* GetThumbnailCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "../caches/thumbnail";
        }

        static void CreateDirectoryIfNeeded(std::string directory)
        {
            if (!std::filesystem::exists(directory))
                std::filesystem::create_directories(directory);
        }
    }
}
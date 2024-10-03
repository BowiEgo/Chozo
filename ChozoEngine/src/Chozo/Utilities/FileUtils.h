#pragma once

namespace Chozo::Utils {

    namespace File {

        static const char* GetAssetDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "../assets";
        }

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
            if (!fs::exists(directory))
                fs::create_directories(directory);
        }

        static void DeleteFile(const std::string& filepath)
        {
            try
            {
                if (fs::exists(filepath))
                {
                    fs::remove(filepath);
                    CZ_CORE_TRACE("File deleted successfully");
                }
                else
                {
                    CZ_CORE_WARN("File not found");
                }
            }
            catch (const fs::filesystem_error& err)
            {
                CZ_CORE_ERROR("Error: {}", err.what());
            }
        }

        static std::string BytesToHumanReadable(uint64_t bytes) {
            const uint64_t KB = 1024;
            const uint64_t MB = KB * 1024;
            const uint64_t GB = MB * 1024;

            std::ostringstream oss;

            if (bytes >= GB) {
                oss << std::fixed << std::setprecision(2) << static_cast<double>(bytes) / GB << " GB";
            } else if (bytes >= MB) {
                oss << std::fixed << std::setprecision(2) << static_cast<double>(bytes) / MB << " MB";
            } else if (bytes >= KB) {
                oss << std::fixed << std::setprecision(2) << static_cast<double>(bytes) / KB << " KB";
            } else {
                oss << bytes << " Bytes";
            }

            return oss.str();
        }
    }
}
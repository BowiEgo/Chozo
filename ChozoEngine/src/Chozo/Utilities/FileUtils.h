#pragma once

#include <regex>

namespace Chozo {

    namespace Utils::File {

        static const std::regex imagePattern(R"(\.(png|jpg|jpeg|hdr)$)", std::regex::icase);
        static const std::regex hdrPattern(R"(\.(hdr)$)", std::regex::icase);
        static const std::regex scenePattern(R"(\.(chozo)$)", std::regex::icase);

        static const bool IsImage(std::string path)
        {
            fs::path filePath = fs::path(path);
            std::string fileExtension = filePath.extension().string();
            return std::regex_match(fileExtension, imagePattern);
        }

        static const char* GetAssetDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "../assets";
        }

        static const char* GetShaderSoureceDirectory()
        {
            return "../resources/shaders";
        }

        static const char* GetShaderCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            return "../caches/shader";
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

        static std::string ReadTextFile(const std::string& filepath)
        {
            std::string result;
            std::ifstream in(filepath, std::ios::in | std::ios::binary);
            if (in)
            {
                in.seekg(0, std::ios::end);
                result.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&result[0], result.size());
                in.close();
            }
            else
            {
                CZ_CORE_ERROR("Could not open file '{0}'", filepath);
            }

            return result;
        }

        static bool ReadBinaryFile(const std::string& filepath, std::vector<u_int32_t>& target)
        {
            std::ifstream in(filepath, std::ios::in | std::ios::binary);
            if (in)
            {
                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                in.seekg(0, std::ios::beg);

                target.resize(size / sizeof(uint32_t));
                in.read((char*)target.data(), size);
                return true;
            }
            else
            {
                CZ_CORE_ERROR("Could not open file '{0}'", filepath);
                return false;
            }
        }

        static void DeleteFile(const std::string& filepath)
        {
            try
            {
                if (fs::exists(filepath))
                {
                    fs::remove(filepath);
                    CZ_CORE_TRACE("File at {} deleted successfully", filepath);
                }
                else
                {
                    CZ_CORE_WARN("File at {} not found", filepath);
                }
            }
            catch (const fs::filesystem_error& err)
            {
                CZ_CORE_ERROR("Error: {}  {}", err.what(), filepath);
            }
        }

        static std::string BytesToHumanReadable(uint64_t bytes)
        {
            const uint64_t KB = 1024;
            const uint64_t MB = KB * 1024;
            const uint64_t GB = MB * 1024;

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);

            if (bytes >= GB) {
                oss << static_cast<double>(bytes) / GB << " GB";
            } else if (bytes >= MB) {
                oss << static_cast<double>(bytes) / MB << " MB";
            } else if (bytes >= KB) {
                oss << static_cast<double>(bytes) / KB << " KB";
            } else {
                oss << bytes << " Bytes";
            }
            return oss.str();
        }
    }
}
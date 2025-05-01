#pragma once

#include <regex>

#if defined(CZ_PLATFORM_WIN)
#include <Windows.h>
#elif defined(CZ_PLATFORM_MACOS)
#include <mach-o/dyld.h>
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

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

        static fs::path GetExecutablePath()
        {
#if defined(CZ_PLATFORM_WIN)
            wchar_t buffer[MAX_PATH] = { 0 };
            GetModuleFileNameW(nullptr, buffer, MAX_PATH);
            return fs::path(buffer).lexically_normal();
#elif defined(CZ_PLATFORM_MACOS)
            char buffer[1024];
            uint32_t size = sizeof(buffer);
            if (_NSGetExecutablePath(buffer, &size) == 0)
                return fs::path(buffer).lexically_normal();
            else {
                std::string path(size, '\0');
                _NSGetExecutablePath(path.data(), &size);
                return fs::path(path).lexically_normal();
            }
#endif
        }

        static fs::path GetAbsolutePath(const fs::path& path)
        {
            fs::path result;

            if (path.is_relative()) {
                fs::path exePath = fs::absolute(Utils::File::GetExecutablePath()).parent_path();
                result = (exePath / path).lexically_normal();
            }

            result = fs::absolute(result).lexically_normal();

            std::wstring dirStr = result.wstring();
            std::replace(dirStr.begin(), dirStr.end(), L'\\', L'/');

            return fs::path(dirStr);
        }

        static bool CreateDirectoryIfNeeded(std::string directory)
        {
            if (!fs::exists(directory))
                return fs::create_directories(directory);

            return false;
        }

        static const fs::path GetResourcesDirectory()
        {
            return GetAbsolutePath(fs::path("../resources"));
        }

        static const fs::path GetShaderSourcesDirectory()
        {
            return GetAbsolutePath(fs::path("../../../../ChozoEngine/shaders"));
        }

        static const fs::path GetAssetDirectory()
        {           
            // TODO: make sure the assets directory is valid
            auto path = GetAbsolutePath(fs::path("./assets"));
            CreateDirectoryIfNeeded(path.string());
            return path;
        }

        static const fs::path GetShaderCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            auto path = GetAbsolutePath(fs::path("./caches/shader"));
            CreateDirectoryIfNeeded(path.string());
            return path;
        }

        static const fs::path GetThumbnailCacheDirectory()
        {
            // TODO: make sure the assets directory is valid
            auto path = GetAbsolutePath(fs::path("./caches/thumbnail"));
            CreateDirectoryIfNeeded(path.string());
            return path;
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

        static bool ReadBinaryFile(const std::string& filepath, std::vector<uint32_t>& target)
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
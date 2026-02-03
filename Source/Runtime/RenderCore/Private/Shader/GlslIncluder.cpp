#include "GlslIncluder.h"
#include "FileUtils.h"
#include "VFS.h"

namespace Chozo {

shaderc_include_result *GlslIncluder::GetInclude(const char *requestedPath,
                                                 shaderc_include_type type,
                                                 const char *requestingPath,
                                                 size_t include_depth) {
    std::string path = requestedPath;

    // If it's a relative path (not starting with a protocol),
    // you might want to resolve it relative to the 'requestingPath'.
    // For now, VFS::Resolve handles our virtual protocols.
    auto physicalPath = VFS::Resolve(path);
    std::string content = Utils::File::ReadTextFile(physicalPath);

    // Handle file not found case to prevent downstream crashes
    if (content.empty() && !std::filesystem::exists(physicalPath)) {
        auto result = new shaderc_include_result;
        result->content = "File not found";
        result->content_length = 14;
        result->source_name = requestedPath;
        result->source_name_length = strlen(requestedPath);
        result->user_data = nullptr;
        return result;
    }

    auto container = new std::pair<std::string, std::string>(path, content);

    auto result = new shaderc_include_result;
    result->source_name = container->first.c_str();
    result->source_name_length = container->first.length();
    result->content = container->second.c_str();
    result->content_length = container->second.length();
    result->user_data = container;

    return result;
}

void GlslIncluder::ReleaseInclude(shaderc_include_result *data) {
    if (data) {
        if (data->user_data) {
            delete static_cast<std::pair<std::string, std::string> *>(
                data->user_data);
        }
        delete data;
    }
}

} // namespace Chozo
#include "GlslIncluder.h"

#include "Chozo/Utilities/FileUtils.h"

#include <iostream>

namespace Chozo {

    GlslIncluder::GlslIncluder(const shaderc_util::FileFinder* file_finder)
		: m_FileFinder(*file_finder)
    {
    }

    shaderc_include_result* GlslIncluder::GetInclude(const char *requestedPath, const shaderc_include_type type, const char *requestingPath, size_t include_depth)
    {
        const fs::path rawPath = (type == shaderc_include_type_relative)
			? m_FileFinder.FindRelativeReadableFilepath(requestingPath, requestedPath)
			: m_FileFinder.FindReadableFilepath(requestedPath);
    	const fs::path requestedFullPath = rawPath.lexically_normal();

    	const bool included = m_IncludedFiles.find(requestedFullPath) != m_IncludedFiles.end();
    	if (!included)
    		m_IncludedFiles.insert(requestedFullPath);

        const auto name = std::string(requestedFullPath);
        const std::string contents = Utils::File::ReadTextFile(name);

        const auto container = new std::array<std::string, 2>;
        (*container)[0] = name;
        (*container)[1] = contents;

        const auto data = new shaderc_include_result;

        data->user_data = container;

        data->source_name = (*container)[0].data();
        data->source_name_length = (*container)[0].size();

    	data->content = included ? nullptr : (*container)[1].data();
        data->content_length = (*container)[1].size();

    	// std::cout << "GetInclude: " << std::endl;
    	// std::cout << requestedFullPath << std::endl;
    	// std::cout << data->source_name << std::endl;
    	// std::cout << data->content << std::endl;

        return data;
    }

    void GlslIncluder::ReleaseInclude(shaderc_include_result *data)
    {
        delete data;
    }

} // namespace Chozo

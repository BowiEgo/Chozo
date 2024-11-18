#pragma once

#include "czpch.h"

#include <shaderc/shaderc.hpp>
#include <libshaderc_util/file_finder.h>

namespace Chozo {
    
    class GlslIncluder : public shaderc::CompileOptions::IncluderInterface
    {
    public:
        explicit GlslIncluder(const shaderc_util::FileFinder* file_finder);

        shaderc_include_result* GetInclude(
            const char* requestedPath,
            shaderc_include_type type,
            const char* requestingPath,
            size_t include_depth
        ) override;

        void ReleaseInclude(shaderc_include_result* data) override;
    private:
		const shaderc_util::FileFinder& m_FileFinder;
    };
} // namespace Chozo

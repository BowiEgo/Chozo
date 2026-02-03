#pragma once

#include <shaderc/shaderc.hpp>

namespace Chozo {

class GlslIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    shaderc_include_result *GetInclude(const char *requestedPath,
                                       shaderc_include_type type,
                                       const char *requestingPath,
                                       size_t include_depth) override;

    void ReleaseInclude(shaderc_include_result *data) override;
};
} // namespace Chozo

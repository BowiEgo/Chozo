#pragma once

#include <shaderc/shaderc.hpp>

class FGlslIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    shaderc_include_result* GetInclude(const char* requestedPath, shaderc_include_type type,
                                       const char* requestingPath, size_t include_depth) override;

    void ReleaseInclude(shaderc_include_result* data) override;

private:
    std::unordered_map<std::string, std::filesystem::path> m_FileCache;
    std::unordered_set<std::filesystem::path> m_IncludedFiles;
    std::vector<std::filesystem::path> m_IncludeStack;
};

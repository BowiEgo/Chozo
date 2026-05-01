#pragma once

#include <shaderc/shaderc.hpp>

class FGlslIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    shaderc_include_result* GetInclude(const char* requestedPath, shaderc_include_type type,
                                       const char* requestingPath, size_t include_depth) override;

    void ReleaseInclude(shaderc_include_result* data) override;

private:
    std::unordered_map<std::string, std::string> m_FileCache; // 文件内容缓存
    std::unordered_set<std::string> m_IncludedFiles;          // 已包含文件集合（防止重复）
    // 可选: 检测递归包含的调用栈
    std::vector<std::string> m_IncludeStack;
};

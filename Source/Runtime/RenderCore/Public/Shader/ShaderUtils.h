#pragma once

#include "ShaderTypes.h"
#include "StringUtils.h"

#include <shaderc/shaderc.hpp>

namespace Chozo {

namespace ShaderUtils {

inline ShaderStage StringToShaderStage(std::string_view shaderStage) {
#define GENERATE_IF(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM)                  \
    if (shaderStage == #ENUM)                                                  \
        return ShaderStage::ENUM;
    FOREACH_SHADER_STAGE(GENERATE_IF)
#undef GENERATE_IF
    return ShaderStage::None;
}

inline const char *ShaderStageToString(ShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM)                \
    case ShaderStage::ENUM:                                                    \
        return #ENUM;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return "Unknown";
    }
}

inline ShaderStage GetShaderStageFromExtension(const std::string &extension) {
    const std::string ext = Utils::String::ToLowerCopy(extension);
    if (s_ShaderExtensionMap.find(ext) == s_ShaderExtensionMap.end())
        return ShaderStage::None;

    return s_ShaderExtensionMap.at(ext);
}

inline shaderc_shader_kind ShaderStageToKind(ShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM)                \
    case ShaderStage::ENUM:                                                    \
        return shaderc_glsl_##LOWER_ENUM##_shader;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return static_cast<shaderc_shader_kind>(0);
    }
}

inline std::string
    ShaderStageToVulkanCacheFileExtension(const ShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM)                \
    case ShaderStage::ENUM:                                                    \
        return ".cache_vulkan." + std::string(#SHORT_ENUM);
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return "";
    }
}

inline fs::path GetCachePathByNameAndStage(const std::string_view name,
                                           ShaderStage stage) {
    if (name.find_first_of("/\\") != std::string_view::npos) {
        throw std::invalid_argument("Shader name contains path separators");
    }

    fs::path cacheDir = Utils::File::GetShaderCacheDirectory();

    fs::path fullPath =
        (cacheDir / name)
            .concat(ShaderUtils::ShaderStageToVulkanCacheFileExtension(stage))
            .lexically_normal();

    std::wstring pathStr = fullPath.wstring();
    std::replace(pathStr.begin(), pathStr.end(), L'\\', L'/');

    return fs::path(pathStr);
}

} // namespace ShaderUtils

} // namespace Chozo

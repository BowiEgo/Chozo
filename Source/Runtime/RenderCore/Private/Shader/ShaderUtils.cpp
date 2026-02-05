#include "ShaderUtils.h"
#include "FileUtils.h"
#include "StringUtils.h"

#include <shaderc/shaderc.hpp>

namespace ChozoUtils::Shader {

namespace {

const std::unordered_map<std::string, EShaderStage> s_ShaderExtensionMap = {
#define GENERATE_MAP(ENUM, LOWER, UPPER, SHORT, GLSL)                          \
    {"." #SHORT, EShaderStage::ENUM},
    FOREACH_SHADER_STAGE(GENERATE_MAP)
#undef GENERATE_MAP
        {".pixel", EShaderStage::Fragment}};

} // namespace

EShaderStage StringToStage(std::string_view shaderStage) {
#define GENERATE_IF(ENUM, LOWER, UPPER, SHORT, GLSL)                           \
    if (shaderStage == #ENUM)                                                  \
        return EShaderStage::ENUM;
    FOREACH_SHADER_STAGE(GENERATE_IF)
#undef GENERATE_IF
    return EShaderStage::None;
}

const char *StageToString(EShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL)                         \
    case EShaderStage::ENUM:                                                   \
        return #ENUM;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return "Unknown";
    }
}

EShaderStage GetStageFromExtension(const std::string &extension) {
    const std::string ext = ChozoUtils::String::ToLowerCopy(extension);
    if (s_ShaderExtensionMap.find(ext) == s_ShaderExtensionMap.end())
        return EShaderStage::None;

    return s_ShaderExtensionMap.at(ext);
}

uint32 StageToKind(EShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL)                         \
    case EShaderStage::ENUM:                                                   \
        return shaderc_glsl_##GLSL##_shader;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return 0;
    }
}

std::string StageToVulkanCacheFileExtension(const EShaderStage shaderStage) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL)                         \
    case EShaderStage::ENUM:                                                   \
        return ".cache_vulkan." + std::string(#SHORT);
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
    default:
        return "";
    }
}

std::filesystem::path GetCachePathByNameAndStage(const std::string_view name,
                                                 EShaderStage stage) {
    if (name.find_first_of("/\\") != std::string_view::npos) {
        throw std::invalid_argument("Shader name contains path separators");
    }

    std::filesystem::path cacheDir =
        ChozoUtils::File::GetShaderCacheDirectory();

    std::filesystem::path fullPath =
        (cacheDir / name)
            .concat(ChozoUtils::Shader::StageToVulkanCacheFileExtension(stage))
            .lexically_normal();

    std::wstring pathStr = fullPath.wstring();
    std::replace(pathStr.begin(), pathStr.end(), L'\\', L'/');

    return std::filesystem::path(pathStr);
}
} // namespace ChozoUtils::Shader
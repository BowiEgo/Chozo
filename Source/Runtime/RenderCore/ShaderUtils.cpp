#include "ShaderUtils.hpp"

#include <Core/Utilities/StringUtils.hpp>
#include <Runtime/RHI/RHITypes.hpp>

#include <string>
#include <unordered_map>

namespace CZ::ShaderUtils {

const std::unordered_map<std::string, ShaderStage> s_ShaderExtensionMap = {
#define GENERATE_MAP(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                        \
    { "." #SHORT, ShaderStage::ENUM },
    FOREACH_SHADER_STAGE(GENERATE_MAP)
#undef GENERATE_MAP
        { ".pixel", ShaderStage::Fragment }
};

ShaderStage StringToStage(std::string_view shaderStage) {
#define GENERATE_IF(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                         \
    if (shaderStage == #ENUM) return ShaderStage::ENUM;
    FOREACH_SHADER_STAGE(GENERATE_IF)
#undef GENERATE_IF
    return ShaderStage::None;
}

const std::string StageToString(ShaderStage shaderStage, bool bUpper) {
    switch (shaderStage) {
#define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER)                       \
    case ShaderStage::ENUM: return bUpper ? #UPPER : #ENUM;
        FOREACH_SHADER_STAGE(GENERATE_CASE)
#undef GENERATE_CASE
        default: return "Unknown";
    }
}

ShaderStage GetStageFromExtension(const std::string& extension) {
    const std::string ext = StringUtils::ToLowerCopy(extension);
    if (s_ShaderExtensionMap.find(ext) == s_ShaderExtensionMap.end()) return ShaderStage::None;

    return s_ShaderExtensionMap.at(ext);
}

ShaderStage GetShaderStageFromSlangStage(const SlangStage slangStage) {
    switch (slangStage) {
        case SLANG_STAGE_VERTEX: return ShaderStage::Vertex;
        case SLANG_STAGE_HULL: return ShaderStage::Hull;
        case SLANG_STAGE_DOMAIN: return ShaderStage::Domain;
        case SLANG_STAGE_GEOMETRY: return ShaderStage::Geometry;
        case SLANG_STAGE_FRAGMENT: return ShaderStage::Fragment;
        case SLANG_STAGE_COMPUTE: return ShaderStage::Compute;
        default: return ShaderStage::None;
    }
}

// #define GENERATE_CASE(ENUM, LOWER, UPPER, SHORT, GLSL, VULKAN, VULKAN_UPPER) \
// #undef GENERATE_CASE

//     // ... 处理 Int / UInt ...

//         // --- 32-bit Float (最常用) ---

//         // --- 32-bit Signed Int ---

//         // --- 32-bit Unsigned Int ---

//         // --- 16-bit Float (Half Float) ---
//         // 如果引擎支持 Half 类型，可以增加对应枚举，否则暂存为 Float

//         // --- 64-bit Float (Double) ---
//         // 通常实时渲染不常用，映射到 None 或根据需要处理

//         case SPV_REFLECT_FORMAT_UNDEFINED:

//         case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:

//         case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//         case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return

//         case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//         case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return

//         default:
//             // CZ_LOG(LogShader, Warning, "Unknown SPIR-V descriptor type: %d", spvType);

} // namespace CZ::ShaderUtils

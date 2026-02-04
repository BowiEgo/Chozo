#pragma once

#include "RHITypes.h"

namespace ChozoUtils::Shader {

EShaderStage StringToShaderStage(std::string_view shaderStage);

const char *ShaderStageToString(EShaderStage shaderStage);

EShaderStage GetShaderStageFromExtension(const std::string &extension);

uint32 ShaderStageToKind(EShaderStage shaderStage);

std::string
    ShaderStageToVulkanCacheFileExtension(const EShaderStage shaderStage);

std::filesystem::path GetCachePathByNameAndStage(const std::string_view name,
                                                 EShaderStage stage);

} // namespace ChozoUtils::Shader

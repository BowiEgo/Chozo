#pragma once

#include "RHITypes.h"

#include <spirv_reflect.h>

namespace ChozoUtils::Shader {

EShaderStage StringToStage(std::string_view shaderStage);

const char* StageToString(EShaderStage shaderStage);

EShaderStage GetStageFromExtension(const std::string& extension);

uint32 StageToKind(EShaderStage shaderStage);

EShaderDataFormat GetDataFormatFromSpv(const SpvReflectTypeDescription& type);
EShaderDataFormat GetDataFormatFromSpv(const SpvReflectFormat& format);

EUniformType GetUniformTypeFromSpvDescType(const SpvReflectDescriptorType& spvType);

// std::string StageToVulkanCacheFileExtension(const EShaderStage shaderStage);

// std::filesystem::path GetCachePathByNameAndStage(const std::string_view name,
//                                                  EShaderStage stage);

} // namespace ChozoUtils::Shader

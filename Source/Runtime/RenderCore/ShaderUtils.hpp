#pragma once

#include <Runtime/RHI/RHITypes.hpp>

#include "slang.h"

namespace CZ::ShaderUtils {

ShaderStage StringToStage(std::string_view shaderStage);

const std::string StageToString(ShaderStage shaderStage, bool bUpper = false);

ShaderStage GetStageFromExtension(const std::string& extension);

ShaderStage GetShaderStageFromSlangStage(const SlangStage slangStage);

//                                                  ShaderStage stage);

} // namespace CZ::ShaderUtils

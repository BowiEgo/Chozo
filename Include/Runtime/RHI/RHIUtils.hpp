#pragma once

#include "RHITypes.hpp"

namespace CZ::RHIUtils {

bool IsDepthFormat(PixelFormat format);

ShaderDataType ToShaderDataFormat(PixelFormat format);

} // namespace CZ::RHIUtils
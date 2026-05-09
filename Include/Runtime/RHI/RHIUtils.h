#pragma once

#include "RHITypes.h"

namespace CZ::RHIUtils {

bool IsDepthFormat(PixelFormat format);

ShaderDataType ToShaderDataFormat(PixelFormat format);

} // namespace CZ::RHIUtils
#include "RHIUtils.h"

namespace ChozoUtils::RHI {

bool IsDepthFormat(EPixelFormat format) {
    return format == EPixelFormat::D32_SFLOAT || format == EPixelFormat::D24_UNORM_S8_UINT ||
           format == EPixelFormat::D16_UNORM;
}

}
#pragma once

#include "CoreMinimal.h"
#include "RHIContext.h"
#include "RHITexture2D.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUIUtils, Info);

namespace ChozoUtils::UI {

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_RED 0xFF0000FF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFFFF0000
#define COLOR_YELLOW 0xFF00FFFF
#define COLOR_CYAN 0xFFFFFF00
#define COLOR_MAGENTA 0xFFFF00FF

TRef<IRHITexture2D> LoadSVGIcon(const IRHIContext* ctx, const std::string& name,
                                int targetSize = 32, uint32 strokeColor = COLOR_WHITE);

unsigned char* LoadImagePreview(const char* path, int max_size, int* out_w, int* out_h);

} // namespace ChozoUtils::UI
#pragma once

#include "RenderCoreExport.h"

#include "Texture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTexture2D, Info);

class RENDER_CORE_API CTexture2D : public CTexture {
public:
    using CTexture::CTexture;
    virtual ~CTexture2D() = default;
};

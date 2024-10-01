#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {
    
    class ThumbnailRenderer : public RefCounted
    {
    public:
        virtual Ref<Texture2D> Render(AssetMetadata metadata, Ref<Asset> asset) const = 0;
    };

    class TextureThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        virtual Ref<Texture2D> Render(AssetMetadata metadata, Ref<Asset> asset) const override;
    };
    
} // namespace Chozo

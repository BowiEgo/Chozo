#include "ThumbnailRenderer.h"

#include "Chozo/FileSystem/TextureExporter.h"

namespace Chozo {

    Ref<Texture2D> TextureThumbnailRenderer::Render(AssetMetadata metadata, Ref<Asset> asset) const
    {
        if (asset->GetAssetType() != AssetType::Texture)
            CZ_CORE_ERROR("[TextureThumbnailRenderer] asset type invalid!");

        Buffer originalBuffer;
        Ref<Texture2D> texture = asset.As<Texture2D>();
        bool isHDR = texture->GetSpecification().Format == ImageFormat::HDR;
        texture->CopyToHostBuffer(originalBuffer);

        uint32_t targetWidth, targetHeight;
        uint32_t originalWidth = texture->GetWidth();
        uint32_t originalHeight = texture->GetHeight();

        if (originalWidth < originalHeight)
        {
            targetHeight = 100;
            targetWidth = static_cast<int>(targetHeight * (static_cast<float>(originalWidth) / originalHeight));
        }
        else
        {
            targetWidth = 100;
            targetHeight = static_cast<int>(targetWidth * (static_cast<float>(originalHeight) / originalWidth));
        }

        std::string filename = std::to_string(metadata.Handle);
        std::filesystem::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        Utils::File::CreateDirectoryIfNeeded(cacheDir.string());

        std::filesystem::path filepath = cacheDir / filename;
        Buffer buffer = TextureExporter::ToFileFromBuffer(
                        filepath,
                        originalBuffer,
                        originalWidth, originalHeight,
                        targetWidth, targetHeight,
                        isHDR);

        Texture2DSpecification spec;
        spec.Width = targetWidth;
        spec.Height = targetHeight;
        spec.Format = ImageFormat::RGBA;

        originalBuffer.Release();

        Ref<Texture2D> outputTexture = Texture2D::Create(buffer, spec);

        return outputTexture;
    }

} // namespace Chozo

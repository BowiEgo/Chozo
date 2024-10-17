#include "ThumbnailPoolTask.h"
#include "ThumbnailRenderer.h"
#include "ThumbnailManager.h"

namespace Chozo {

    void ThumbnailPoolTask::Execute()
    {
        ThumbnailRenderer::RenderTask(this);
    }

    void ThumbnailPoolTask::Finish()
    {
        glm::vec2 srcSize;
        glm::vec2 outputSize(ThumbnailRenderer::GetThumbnailSize(), ThumbnailRenderer::GetThumbnailSize());
        bool isHDR = false;

        if (Source->GetAssetType() == AssetType::Texture)
        {
            auto src = Source.As<Texture2D>();
            isHDR = src->GetSpecification().Format == ImageFormat::HDR;
            srcSize.x = src->GetWidth();
            srcSize.y = src->GetHeight();

            if (srcSize.x < srcSize.y)
                outputSize.x = static_cast<int>(outputSize.y * (static_cast<float>(srcSize.x) / srcSize.y));
            else
                outputSize.y = static_cast<int>(outputSize.x * (static_cast<float>(srcSize.y) / srcSize.x));
        }
        else
        {
            auto vpSize = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()->GetViewportSize();
            srcSize.x = vpSize.x;
            srcSize.y = vpSize.y;
        }

        if (Flags & PoolTaskFlags_Export)
        {
            std::string filename = std::to_string(Source->Handle);
            Utils::ExportPNG(filename, ImageData.Data,
                srcSize,
                outputSize,
                isHDR);
        }

        Texture2DSpecification spec;
        spec.Width = srcSize.x;
        spec.Height = srcSize.y;
        spec.Format = ImageFormat::RGBA;
        auto thumbnail = Texture2D::Create(ImageData, spec);
        ThumbnailManager::SetThumbnail(Source->Handle, thumbnail);

        ImageData.Release();
    }

} // namespace Chozo


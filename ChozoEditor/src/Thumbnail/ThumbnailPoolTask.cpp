#include "ThumbnailPoolTask.h"
#include "ThumbnailRenderer.h"
#include "ThumbnailManager.h"

namespace Chozo {

    void ThumbnailPoolTask::Execute()
    {
        SetStatus(TaskStatus::Executed); // Right now SetStatus should before RenderTask.
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
            srcSize.x = static_cast<float>(src->GetWidth());
            srcSize.y = static_cast<float>(src->GetHeight());

            if (srcSize.x < srcSize.y)
                outputSize.x = outputSize.y * (srcSize.x / srcSize.y);
            else
                outputSize.y = outputSize.x * (srcSize.y / srcSize.x);
        }
        else
        {
            auto vpSize = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()->GetViewportSize();
            srcSize.x = vpSize.x;
            srcSize.y = vpSize.y;
        }

        if (Flags & PoolTaskFlags_Export)
        {
            const std::string filename = std::to_string(Source->Handle);
            Utils::ExportPNG(filename, ImageData.Data,
                srcSize,
                outputSize,
                isHDR);
        }

        Texture2DSpecification spec;
        spec.Width = static_cast<uint32_t>(srcSize.x);
        spec.Height = static_cast<uint32_t>(srcSize.y);
        spec.Format = ImageFormat::RGBA;
        const auto thumbnail = Texture2D::Create(ImageData, spec); // NOLINT
        ThumbnailManager::SetThumbnail(Source->Handle, thumbnail);

        ImageData.Release();
    }

} // namespace Chozo


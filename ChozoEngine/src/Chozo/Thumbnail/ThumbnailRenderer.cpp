#include "ThumbnailRenderer.h"

#include "Chozo/FileSystem/TextureExporter.h"
#include "Chozo/Scene/Entity.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"

namespace Chozo {

    Ref<Texture2D> TextureThumbnailRenderer::Render(AssetMetadata metadata, Ref<Asset> asset)
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
        fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        Utils::File::CreateDirectoryIfNeeded(cacheDir.string());

        fs::path filepath = cacheDir / filename;
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

    //==============================================================================
    /// MaterialThumbnailRenderer
    MaterialThumbnailRenderer::MaterialThumbnailRenderer()
    {
        m_Scene = Ref<Scene>::Create();
		m_SceneRenderer = SceneRenderer::Create(m_Scene);
        m_SceneRenderer->SetActive(true);
        m_SceneRenderer->SetViewportSize(100, 100);
        m_Camera = EditorCamera(12.0f, 1.0f, 0.1f, 1000.0f);

        auto sphere = m_Scene->CreateEntity("Sphere");
        Ref<Geometry> geom = Ref<SphereGeometry>::Create();
        sphere.AddComponent<MeshComponent>(geom);

        auto pointLight = m_Scene->CreateEntity("Point Light");
        pointLight.AddComponent<PointLightComponent>();
        pointLight.GetComponent<TransformComponent>().Translation.x = 2.0f;
        pointLight.GetComponent<TransformComponent>().Translation.y = 2.0f;
        pointLight.GetComponent<TransformComponent>().Translation.z = 2.0f;
    }

    Ref<Texture2D> MaterialThumbnailRenderer::Render(AssetMetadata metadata, Ref<Asset> asset)
    {
        if (asset->GetAssetType() != AssetType::Material)
            CZ_CORE_ERROR("[MaterialThumbnailRenderer] asset type invalid!");

        Ref<Texture2D> texture = GetOutput();

        Buffer originalBuffer;
        texture->CopyToHostBuffer(originalBuffer);

        std::string filename = std::to_string(metadata.Handle);
        fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        Utils::File::CreateDirectoryIfNeeded(cacheDir.string());

        fs::path filepath = cacheDir / filename;
        Buffer buffer = TextureExporter::ToFileFromBuffer(
                        filepath,
                        originalBuffer,
                        100, 100,
                        100, 100,
                        false);

        originalBuffer.Release();

        return texture;
    }

    void MaterialThumbnailRenderer::OnUpdate()
    {
        m_Scene->OnUpdateEditor(0, m_Camera);
    }

    Entity MaterialThumbnailRenderer::GetSphere()
    {
        auto view = m_Scene->Reg().view<TransformComponent, MeshComponent>();
        for (auto entity : view)
        {
            return Entity{ entity, m_Scene.get() };
        }

        return Entity();
    }
} // namespace Chozo

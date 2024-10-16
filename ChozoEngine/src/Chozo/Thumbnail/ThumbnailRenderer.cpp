#include "ThumbnailRenderer.h"

#include "Chozo/Core/Application.h"
#include "Chozo/FileSystem/TextureExporter.h"
#include "Chozo/Scene/Entity.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"

#include <glad/glad.h>

namespace Chozo {

	std::unordered_map<AssetType, Scope<ThumbnailRenderer>> ThumbnailRenderer::s_Renderers;
    uint32_t ThumbnailRenderer::s_ThumbnailSize = 200;

    using Size = std::pair<uint32_t, uint32_t>;

    namespace Utils {

        static void ExportPNG(std::string filename, Buffer source, Size srcSize, Size outputSize, bool isHDR)
        {
            fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
            fs::path filepath = cacheDir / filename;

            Utils::File::CreateDirectoryIfNeeded(cacheDir.string());

            Buffer buffer = TextureExporter::ToFileFromBuffer(
                            filepath,
                            source,
                            srcSize.first, srcSize.second,
                            outputSize.first, outputSize.second,
                            isHDR);
        }
    }

    void ThumbnailRenderer::Init()
    {
        s_Renderers.clear();
        for (int i = 0; i < static_cast<int>(AssetType::None); i++)
        {
            AssetType type = static_cast<AssetType>(i);
            switch (type)
            {
            case AssetType::Texture:
                s_Renderers[type] = CreateScope<TextureThumbnailRenderer>(); break;
            case AssetType::Material:
                s_Renderers[type] = CreateScope<MaterialThumbnailRenderer>(); break;
            default:
                break;
            }
        }
    }

    void ThumbnailRenderer::RenderTask(Ref<ThumbnailPoolTask> task)
    {
        Ref<Asset> asset = task->Source;
        auto renderer = GetRenderer(asset->GetAssetType());
        renderer->Render(task);
    }

    template <>
    TextureThumbnailRenderer* ThumbnailRenderer::GetRenderer<TextureThumbnailRenderer>()
    {
        if (s_Renderers[AssetType::Texture])
            return static_cast<TextureThumbnailRenderer*>(s_Renderers[AssetType::Texture].get());
        else
            return nullptr;
    }

    template <>
    MaterialThumbnailRenderer* ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()
    {
        if (s_Renderers[AssetType::Material])
            return static_cast<MaterialThumbnailRenderer*>(s_Renderers[AssetType::Material].get());
        else
            return nullptr;
    }

    void TextureThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
        Ref<Asset> asset = task->Source;
        AssetHandle handle = asset->Handle;

        Buffer srcBuffer;
        Ref<Texture2D> src = asset.As<Texture2D>();
        src->CopyToHostBuffer(srcBuffer);

        Size srcSize(src->GetWidth(), src->GetHeight());
        Size outputSize(0, 0);
        auto [srcWidth, srcHeight] = srcSize;

        if (srcWidth < srcHeight)
        {
            outputSize.second = ThumbnailRenderer::GetThumbnailSize(); // TODO: Move to Config
            outputSize.first = static_cast<int>(outputSize.second * (static_cast<float>(srcWidth) / srcHeight));
        }
        else
        {
            outputSize.first = ThumbnailRenderer::GetThumbnailSize();
            outputSize.second = static_cast<int>(outputSize.first * (static_cast<float>(srcHeight) / srcWidth));
        }

        if (task->Flags & TaskFlags_Export)
        {
            bool isHDR = src->GetSpecification().Format == ImageFormat::HDR;
            std::string filename = std::to_string(handle);
            
            Utils::ExportPNG(filename, srcBuffer, srcSize, outputSize, isHDR);
        }

        Texture2DSpecification spec;
        spec.Width = srcSize.first;
        spec.Height = srcSize.second;
        spec.Format = ImageFormat::RGBA;
        task->Thumbnail = Texture2D::Create(srcBuffer, spec);

        task->Status = TaskStatus::Finished;
    }

    //==============================================================================
    /// MaterialThumbnailRenderer
    uint32_t MaterialThumbnailRenderer::s_MaterialThumbnailSize = 200;

    MaterialThumbnailRenderer::MaterialThumbnailRenderer()
    {
        m_Scene = Ref<Scene>::Create();
		m_SceneRenderer = SceneRenderer::Create(m_Scene);
        m_SceneRenderer->SetActive(true);
        m_SceneRenderer->SetViewportSize(s_MaterialThumbnailSize, s_MaterialThumbnailSize);
        m_Camera = EditorCamera(12.0f, 1.0f, 0.1f, 1000.0f);

        auto sphere = m_Scene->CreateEntity("Sphere");
        Ref<Geometry> geom = Ref<SphereGeometry>::Create();
        m_Material = Material::Create("PBR");
        Application::GetAssetManager()->AddMemoryOnlyAsset(m_Material);
        sphere.AddComponent<MeshComponent>(geom, MeshType::Dynamic, m_Material->Handle);

        auto dirLight = m_Scene->CreateEntity("Directional Light");
        dirLight.AddComponent<DirectionalLightComponent>();
        dirLight.GetComponent<DirectionalLightComponent>().Direction = glm::vec3(-45.0f, 45.0f, 45.0f);
        dirLight.GetComponent<DirectionalLightComponent>().Intensity = 2.0f;

        auto pointLight = m_Scene->CreateEntity("Point Light");
        pointLight.AddComponent<PointLightComponent>();
        pointLight.GetComponent<TransformComponent>().Translation.x = 2.0f;
        pointLight.GetComponent<TransformComponent>().Translation.y = 2.0f;
        pointLight.GetComponent<TransformComponent>().Translation.z = 2.0f;
    }

    void MaterialThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
        Ref<Asset> asset = task->Source;
        AssetHandle handle = asset->Handle;

        SetMaterial(asset.As<Material>());

        RenderToBuffer([task, handle, this](Buffer output) mutable {
            fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
            Utils::File::CreateDirectoryIfNeeded(cacheDir.string());

            Size srcSize(s_MaterialThumbnailSize, s_MaterialThumbnailSize);
            Size outputSize(ThumbnailRenderer::GetThumbnailSize(), ThumbnailRenderer::GetThumbnailSize());

            if (task->Flags & TaskFlags_Export)
            {
                std::string filename = std::to_string(handle);
                Utils::ExportPNG(filename, output,
                    srcSize,
                    outputSize,
                    false);
            }

            Texture2DSpecification spec;
            spec.Width = srcSize.first;
            spec.Height = srcSize.second;
            spec.Format = ImageFormat::RGBA;
            task->Thumbnail = Texture2D::Create(output, spec);

            task->Status = TaskStatus::Finished;
        });
    }

    void MaterialThumbnailRenderer::SetMaterial(Ref<Material> material)
    {
        auto sphere = GetSphere();
        m_Material->CopyProperties(material);
        // OnUpdate();
    }

    void MaterialThumbnailRenderer::SetMaterialValue(Ref<Material> material, std::string name, UniformValue value)
    {
        auto sphere = GetSphere();

        // if (m_Material->GetShader() != material->GetShader())
        //     sphere.GetComponent<MeshComponent>().MaterialInstance = Material::Copy(material);

        m_Material->Set(name, value);
        OnUpdate();
    }

    void MaterialThumbnailRenderer::SetMaterialValue(Ref<Material> material, std::string name, Ref<Texture2D> texture)
    {
        auto sphere = GetSphere();

        m_Material->Set(name, texture);
        OnUpdate();
    }

    void MaterialThumbnailRenderer::OnUpdate()
    {
        m_Scene->OnUpdateEditor(0, m_Camera);
    }

    void MaterialThumbnailRenderer::RenderToBuffer(Callback<void, const Buffer&> callback)
    {
        m_Scene->RenderToBuffer(m_Camera, callback);
    }

    Entity MaterialThumbnailRenderer::GetSphere()
    {
        auto view = m_Scene->Reg().view<TransformComponent, MeshComponent>();
        for (auto entity : view)
            return Entity{ entity, m_Scene.get() };

        return Entity();
    }

} // namespace Chozo

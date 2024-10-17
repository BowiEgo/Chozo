#include "ThumbnailRenderer.h"

#include "Chozo/Core/Application.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"
#include "Chozo/Scene/Entity.h"
#include "Chozo/FileSystem/TextureExporter.h"
#include "Chozo/Utilities/FileUtils.h"

#include <glad/glad.h>

namespace Chozo {

	std::unordered_map<AssetType, Scope<ThumbnailRenderer>> ThumbnailRenderer::s_Renderers;
    uint32_t ThumbnailRenderer::s_ThumbnailSize = 200;

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

    //==============================================================================
    /// TextureThumbnailRenderer
    void TextureThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
        Ref<Texture2D> src = task->Source.As<Texture2D>();
        src->CopyToHostBuffer(task->ImageData);
    }

    //==============================================================================
    /// MaterialThumbnailRenderer
    MaterialThumbnailRenderer::MaterialThumbnailRenderer()
    {
        m_Scene = Ref<Scene>::Create();
		m_SceneRenderer = SceneRenderer::Create(m_Scene);
        m_SceneRenderer->SetActive(true);
        m_SceneRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
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
        m_SceneRenderer->AddEventListener(
            EventType::AppRender,
            [task, this](Event& e) mutable -> bool {
                CZ_CORE_INFO("Execute");
                SetMaterial(task->Source.As<Material>());
                m_SceneRenderer->CopyCompositeImage(task->ImageData);
                return true;
            },
            true
        );
        OnUpdate();
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

    Entity MaterialThumbnailRenderer::GetSphere()
    {
        auto view = m_Scene->Reg().view<TransformComponent, MeshComponent>();
        for (auto entity : view)
            return Entity{ entity, m_Scene.get() };

        return Entity();
    }

} // namespace Chozo

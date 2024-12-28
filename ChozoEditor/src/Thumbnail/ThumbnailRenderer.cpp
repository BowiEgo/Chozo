#include "ThumbnailRenderer.h"

#include "Chozo/Core/Application.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"
#include "Chozo/Scene/Entity.h"

namespace Chozo {

	std::unordered_map<AssetType, Scope<ThumbnailRenderer>> ThumbnailRenderer::s_Renderers;
    uint32_t ThumbnailRenderer::s_ThumbnailSize = 200;

    void ThumbnailRenderer::Init()
    {
        s_Renderers.clear();
        s_Renderers.emplace(AssetType::Scene, CreateScope<SceneThumbnailRenderer>());
        s_Renderers.emplace(AssetType::Texture, CreateScope<TextureThumbnailRenderer>());
        s_Renderers.emplace(AssetType::Material, CreateScope<MaterialThumbnailRenderer>());
        s_Renderers.emplace(AssetType::MeshSource, CreateScope<MeshSourceThumbnailRenderer>());
    }

    void ThumbnailRenderer::Shutdown()
    {
        s_Renderers.clear();
    }

    void ThumbnailRenderer::RenderTask(Ref<ThumbnailPoolTask> task)
    {
        Ref<Asset> asset = task->Source;
        const auto renderer = GetRenderer(asset->GetAssetType());
        renderer->Render(task);
    }

    template <>
    SceneThumbnailRenderer* ThumbnailRenderer::GetRenderer<SceneThumbnailRenderer>()
    {
        if (s_Renderers[AssetType::Scene])
            return dynamic_cast<SceneThumbnailRenderer*>(s_Renderers[AssetType::Scene].get());

        return nullptr;
    }

    template <>
    TextureThumbnailRenderer* ThumbnailRenderer::GetRenderer<TextureThumbnailRenderer>()
    {
        if (s_Renderers[AssetType::Texture])
            return dynamic_cast<TextureThumbnailRenderer*>(s_Renderers[AssetType::Texture].get());

        return nullptr;
    }

    template <>
    MaterialThumbnailRenderer* ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()
    {
        if (s_Renderers[AssetType::Material])
            return dynamic_cast<MaterialThumbnailRenderer*>(s_Renderers[AssetType::Material].get());

        return nullptr;
    }

    template <>
    MeshSourceThumbnailRenderer* ThumbnailRenderer::GetRenderer<MeshSourceThumbnailRenderer>()
    {
        if (s_Renderers[AssetType::MeshSource])
            return dynamic_cast<MeshSourceThumbnailRenderer*>(s_Renderers[AssetType::MeshSource].get());

        return nullptr;
    }


    //==============================================================================
    // SceneThumbnailRenderer
    SceneThumbnailRenderer::SceneThumbnailRenderer()
    {
		m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene);
        m_SceneRenderer->SetActive(true);
        m_SceneRenderer->SetViewportSize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
        m_Camera = EditorCamera(6.0f, 1.0f, 0.1f, 1000.0f);
    }

    void SceneThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
#if 0
        m_Scene = task->Source.As<Scene>();
        m_SceneRenderer->SetScene(m_Scene);

        m_SceneRenderer->AddEventListener(
            EventType::SceneRender,
            [&task, this](Event& e) -> bool {
                m_SceneRenderer->CopyImage(
                    m_SceneRenderer->GetCompositePass()->GetOutput(0),
                    task->ImageData);
                task->SetStatus(TaskStatus::Finished);
                return true;
            },
            true
        );

        m_Scene->OnUpdateEditor(0, m_Camera);
#else
        Renderer::GetRendererData().WhiteTexture->CopyToHostBuffer(task->ImageData);
        task->SetStatus(TaskStatus::Finished);
#endif
    }

    //==============================================================================
    // TextureThumbnailRenderer
    void TextureThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
        Ref<Texture2D> src = task->Source.As<Texture2D>();
        src->CopyToHostBuffer(task->ImageData);
        task->SetStatus(TaskStatus::Finished);
    }

    //==============================================================================
    // MaterialThumbnailRenderer
    MaterialThumbnailRenderer::MaterialThumbnailRenderer()
    {
        m_Scene = Ref<Scene>::Create();
		m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene);
        m_SceneRenderer->SetActive(true);
        m_SceneRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        m_Camera = EditorCamera(12.0f, 1.0f, 0.1f, 1000.0f);

        auto sphere = m_Scene->CreateEntity("Sphere");
        Ref<Geometry> geom = Geometry::Create<SphereGeometry>();
        m_Material = Material::Create("Lit");
        Application::GetAssetManager()->AddMemoryOnlyAsset(m_Material);
        sphere.AddComponent<MeshComponent>(geom, 0, m_Material->Handle);

        auto dirLight = m_Scene->CreateEntity("Directional Light");
        dirLight.AddComponent<DirectionalLightComponent>();
        dirLight.GetComponent<DirectionalLightComponent>().Direction = glm::vec3(-45.0f, 45.0f, 45.0f);
        dirLight.GetComponent<DirectionalLightComponent>().Intensity = 2.0f;

        auto pointLight = m_Scene->CreateEntity("Point Light");
        pointLight.AddComponent<PointLightComponent>();
        pointLight.GetComponent<TransformComponent>().Translation.x = 2.0f;
        pointLight.GetComponent<TransformComponent>().Translation.y = 2.0f;
        pointLight.GetComponent<TransformComponent>().Translation.z = 2.0f;

        Update();
    }

    void MaterialThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
        m_Updated = false;

        m_SceneRenderer->AddEventListener(
            EventType::SceneRender,
            [&task, this](Event& e) -> bool {
                SetMaterial(task->Source.As<Material>());
                m_SceneRenderer->CopyImage(
                    m_SceneRenderer->GetCompositePass()->GetOutput(0),
                    task->ImageData);
                task->SetStatus(TaskStatus::Finished);
                return true;
            },
            true
        );
        Update();
    }

    void MaterialThumbnailRenderer::Update()
    {
        m_Scene->OnRenderEditor(m_SceneRenderer, 0, m_Camera);
        m_Updated = true;
    }

    void MaterialThumbnailRenderer::SetMaterial(const Ref<Material>& material)
    {
        auto sphere = GetSphere();
        m_Material->CopyProperties(material);
    }

    void MaterialThumbnailRenderer::SetMaterialProp(const std::string& name, const MaterialProp& value)
    {
        std::visit([&](const auto& v) {
            m_Material->Set(name, v);
        }, value);

        Update();
    }

    void MaterialThumbnailRenderer::UpdateCache()
    {
        auto compositeImage = m_SceneRenderer->GetCompositePass()->GetOutput(0);
        compositeImage->ExtractBuffer();
        m_Cache = Texture2D::Create(compositeImage);
    }

    Entity MaterialThumbnailRenderer::GetSphere()
    {
        const auto view = m_Scene->Reg().view<TransformComponent, MeshComponent>();
        for (const auto entity : view)
            return Entity{ entity, m_Scene.get() };

        return {};
    }

    //==============================================================================
    // MeshSourceThumbnailRenderer
    MeshSourceThumbnailRenderer::MeshSourceThumbnailRenderer()
    {
        m_SceneRenderer = Ref<SceneRenderer>::Create(m_Scene);
        m_SceneRenderer->SetActive(true);
        m_SceneRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        m_Camera = EditorCamera(6.0f, 1.0f, 0.1f, 1000.0f);
    }

    void MeshSourceThumbnailRenderer::Render(Ref<ThumbnailPoolTask> task)
    {
        Renderer::GetRendererData().WhiteTexture->CopyToHostBuffer(task->ImageData);
        task->SetStatus(TaskStatus::Finished);
    }

} // namespace Chozo

#include "Scene.h"

#include "Entity.h"
#include "Components.h"
#include "ScriptableEntity.h"
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"

#include <glad/glad.h>

namespace Chozo {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tag  = entity.AddComponent<TagComponent>();

        tag.Tag = name.empty() ? "Entity" : name;

        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
        Ref<Framebuffer> FBO = m_FinalPipeline->GetTargetFramebuffer();
        FBO->Bind();
        Renderer2D::ResetStats();
        Renderer::ResetStats();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();
        // std::vector<int> clearValues(FBO->GetSpecification().Width * FBO->GetSpecification().Height, -1);
        FBO->ClearColorAttachmentBuffer(1); // clear entity ID attachment to -1

        // Skylight
        {
            auto group = m_Registry.group<SkyLightComponent>(entt::get<TransformComponent>);

            for (auto entity : group)
            {
                auto [transform, skyLight] = group.get<TransformComponent, SkyLightComponent>(entity);

                if (!skyLight.SceneEnvironment && skyLight.DynamicSky)
                {
                    Ref<TextureCube> preethamEnv = Renderer::GetRendererData().m_PreethamSkyRenderPass->GetOutput("EnvMap");
                    skyLight.SceneEnvironment = std::make_shared<Environment>(preethamEnv, preethamEnv);
                    RenderCommand::RenderPreethamSky(skyLight.TurbidityAzimuthInclination.x, skyLight.TurbidityAzimuthInclination.y, skyLight.TurbidityAzimuthInclination.z);
                }
                m_Environment = skyLight.SceneEnvironment;
                m_EnvironmentIntensity = skyLight.Intensity;
                m_SkyboxLod = skyLight.Lod;

                Renderer::DrawSkyLight(m_Environment, m_EnvironmentIntensity, m_SkyboxLod, camera);
            }
        }

        // Directional light
        {
            auto view = m_Registry.view<TransformComponent, DirectionalLightComponent>();
            for (auto entity : view)
            {
                if (!m_Registry.valid(entity))
                    continue;

                const auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                Renderer::SubmitDirectionalLight(&light);
            }
        }

        // 3D Renderer
        FBO->Bind();
        Renderer::BeginScene(camera);
        // Renderer::BeginBatch();

        // Draw meshes
        {
            auto view = m_Registry.view<TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                if (!m_Registry.valid(entity))
                    continue;

                const auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

                if (mesh.Type == MeshType::Dynamic)
                {
                    DynamicMesh* dynamicMesh = dynamic_cast<DynamicMesh*>(mesh.MeshInstance.get());
                    if (dynamicMesh->GetMeshSource()->IsBufferChanged())
                        dynamicMesh->Init();
                    Renderer::DrawMesh(transform.GetTransform(), dynamicMesh, mesh.MaterialInstance.get(), (uint32_t)entity);
                }
                else if (mesh.Type == MeshType::Static)
                {
                    StaticMesh* staticMesh = dynamic_cast<StaticMesh*>(mesh.MeshInstance.get());
                    staticMesh->GetMeshSource()->SetLocalTransform(transform.GetTransform());
                    if (staticMesh->GetMeshSource()->IsBufferChanged())
                        Renderer::SubmitStaticMesh(staticMesh);
                }
            }
        }

        Renderer::EndScene();

        // 2D Renderer
        Renderer2D::BeginScene(camera);
        Renderer2D::BeginBatch();

        // Draw sprites
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
            {
                if (!m_Registry.valid(entity))
                    continue;
            
                const auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                // Renderer2D::DrawSprite(transform.GetTransform(), sprite, (uint32_t)entity);
                Renderer2D::DrawRect(transform.GetTransform(), sprite.Color, (uint32_t)entity);
            }
        }

        // Draw circles
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view)
            {
                if (!m_Registry.valid(entity))
                    continue;
            
                const auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (uint32_t)entity);
            }
        }

        Renderer2D::EndScene();
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        Ref<Framebuffer> FBO = m_FinalPipeline->GetTargetFramebuffer();
        FBO->Bind();
        Renderer2D::ResetStats();
        Renderer::ResetStats();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();
        int clearValue = -1;
        FBO->ClearColorAttachmentBuffer(1); // clear entity ID attachment to -1
        
        // Update scripts
        {
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
            {
                // TODO: Move to Scene::OnScenePlay
                if (!nsc.Instance)
                {
                    nsc.Instance = nsc.InstantiateScript();
                    nsc.Instance->m_Entity = Entity{ entity, this };

                    nsc.Instance->OnCreate();
                }

                nsc.Instance->OnUpdate(ts);
            });
        }

        // // Skylight
        // {
        //     auto group = m_Registry.group<SkyLightComponent>(entt::get<TransformComponent>);

        //     for (auto entity : group)
        //     {
        //         auto [transform, skyLight] = group.get<TransformComponent, SkyLightComponent>(entity);

        //         if (!skyLight.SceneEnvironment && skyLight.DynamicSky)
        //         {
        //             Ref<TextureCube> preethamEnv = Renderer::CreatePreethamSky(skyLight.TurbidityAzimuthInclination.x, skyLight.TurbidityAzimuthInclination.y, skyLight.TurbidityAzimuthInclination.z);
        //             skyLight.SceneEnvironment = std::make_shared<Environment>(preethamEnv, preethamEnv);
        //             m_Environment = skyLight.SceneEnvironment;
        //             m_EnvironmentIntensity = skyLight.Intensity;
        //             m_SkyboxLod = skyLight.Lod;
        //         }

        //         Renderer::DrawSkyLight(m_Environment, m_EnvironmentIntensity, m_SkyboxLod);
        //     }
        // }

        // Render 2D scene
        {
            Camera* mainCamera = nullptr;
            glm::mat4 cameraTransform;
            {
                auto view = m_Registry.view<TransformComponent, CameraComponent>();
                for (auto entity : view)
                {
                    const auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                    if (camera.Primary)
                    {
                        mainCamera = &camera.Camera;
                        cameraTransform = transform.GetTransform();
                        break;
                    }
                }
            }

            if (mainCamera)
            {
                Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);
                Renderer2D::BeginBatch();

                // Draw sprites
                {
                    auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                    for (auto entity : group)
                    {
                        if (!m_Registry.valid(entity))
                            continue;
                    
                        const auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                        Renderer2D::DrawSprite(transform.GetTransform(), sprite, (uint32_t)entity);
                    }
                }

                // Draw circles
                {
                    auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                    for (auto entity : view)
                    {
                        if (!m_Registry.valid(entity))
                            continue;
                    
                        const auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                        Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (uint32_t)entity);
                    }
                }

                Renderer2D::EndScene();
            }
        }
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }
    }

    // void Scene::DrawIDBuffer(Ref<Framebuffer> target, EditorCamera& camera)
    // {
    //     target->Bind();
    //     {
    //         // Render to ID buffer
    //         Renderer2D::BeginScene(camera);
    //         Renderer2D::BeginBatch();

    //         auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    //         for (auto entity : group)
    //         {
    //             if (!m_Registry.valid(entity))
    //                 continue;
            
    //             const auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
    //             Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color, (uint32_t)entity);
    //         }

    //         Renderer2D::EndScene();
    //     }
    //     target->Unbind();
    // }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.Primary)
                return Entity{ entity, this };
        }
        return {};
    }

    template<class T>
    struct always_false : std::false_type {};

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        static_assert(always_false<T>::value, "Component type not supported");
    }

    template<>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }

    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<SkyLightComponent>(Entity entity, SkyLightComponent& component)
    {
    }

    template<>
    void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
    {
    }
}
#include "Scene.h"

#include "Entity.h"
#include "Components.h"
#include "Chozo/Renderer/Renderer2D.h"

namespace Chozo {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        
        Entity entity = { m_Registry.create(), this };
        entity.AddCompoent<TransformComponent>();
        auto& tag  = entity.AddCompoent<TagComponent>();

        tag.Tag = name.empty() ? "Entity" : name;
        
        return entity;
    }

    void Scene::OnUpdate(Timestep ts)
    {
        Camera* mainCamera = nullptr;
        glm::mat4* cameraTransform = nullptr;
        {
            auto view = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                const auto& [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = &transform.Transform;
                    break;
                }
            }
        }

        if (mainCamera)
        {
            Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);
            Renderer2D::BeginBatch();

            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
            {
                const auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

                Renderer2D::DrawQuad(transform, sprite.Color);
            }

            Renderer2D::EndBatch();
            Renderer2D::EndScene();
        }
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComp = view.get<CameraComponent>(entity);
            if (!cameraComp.FixedAspectRatio)
                cameraComp.Camera.SetViewportSize(width, height);
        }
    }
}
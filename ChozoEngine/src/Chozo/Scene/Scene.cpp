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
            auto group = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : group)
            {
                const auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);
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
}
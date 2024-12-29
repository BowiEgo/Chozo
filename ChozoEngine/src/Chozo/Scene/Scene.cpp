#include "Scene.h"

#include "Entity.h"
#include "Components.h"
#include "ScriptableEntity.h"
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"
#include "Chozo/Renderer/SceneRenderer.h"
#include "Chozo/Asset/AssetManager.h"
#include "Chozo/Core/Application.h"

namespace Chozo {

    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        HandleModified();
		return CreateChildEntity({}, name);
    }

    Entity Scene::CreateChildEntity(Entity parent, const std::string &name)
    {
        Entity entity = { m_Registry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>(UUID());
		entity.AddComponent<TransformComponent>();
        auto& tag  = entity.AddComponent<TagComponent>();

        tag.Tag = name.empty() ? "Entity" : name;

		entity.AddComponent<RelationshipComponent>();

        if (parent)
            entity.SetParent(parent);

		m_EntityIDMap[idComponent.ID] = entity;

		SortEntities();
        
        return entity;
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tag  = entity.AddComponent<TagComponent>();

        tag.Tag = name.empty() ? "Entity" : name;

		entity.AddComponent<RelationshipComponent>();

        return entity;
    }

    Entity Scene::GetEntityWithUUID(UUID uuid)
    {
        if (const auto iter = m_EntityIDMap.find(uuid); iter != m_EntityIDMap.end())
			return iter->second;
		return Entity{};
    }

    glm::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity)
    {
		glm::mat4 transform(1.0f);

		Entity parent = GetEntityWithUUID(entity.GetParentUUID());
		if (parent)
            transform = GetWorldSpaceTransformMatrix(parent);

		return transform * entity.Transform().GetTransform();
    }

    bool Scene::EntityExists(entt::entity entity)
    {
        return m_Registry.valid(entity);
    }

    Entity Scene::InstantiateMesh(Ref<Mesh> mesh)
    {
		auto& assetData = Application::GetAssetManager()->GetMetadata(mesh->GetMeshSource()->Handle);
		Entity rootEntity = CreateEntity(assetData.FilePath.stem().string());
		BuildMeshEntityHierarchy(rootEntity, mesh, mesh->GetMeshSource()->GetRootNode());

        return rootEntity;
    }

    void Scene::BuildMeshEntityHierarchy(Entity parent, Ref<Mesh> mesh, const MeshNode &node)
    {
		Ref<MeshSource> meshSource = mesh->GetMeshSource();
		const auto& nodes = meshSource->GetNodes();

		if (node.IsRoot() && node.Submeshes.size() == 0)
        {
            for (uint32_t child : node.Children)
				BuildMeshEntityHierarchy(parent, mesh, nodes[child]);

			return;
        }

		Entity nodeEntity = CreateChildEntity(parent, node.Name);
		nodeEntity.Transform().SetTransform(node.LocalTransform);

        auto size = node.Submeshes.size();

        if (node.Submeshes.size() == 1)
        {
            uint32_t submeshIndex = node.Submeshes[0];
            nodeEntity.AddComponent<MeshComponent>(mesh, submeshIndex);
        }
        else if (node.Submeshes.size() > 1)
        {
            for (uint32_t i = 0; i < size; i++)
            {
				uint32_t submeshIndex = node.Submeshes[i];
				Entity childEntity = CreateChildEntity(nodeEntity, node.Name);
				childEntity.AddComponent<MeshComponent>(mesh, submeshIndex);
            }
        }

        for (uint32_t child : node.Children)
            BuildMeshEntityHierarchy(nodeEntity, mesh, nodes[child]);
    }

    void Scene::SortEntities()
    {
        m_Registry.sort<IDComponent>([&](const auto lhs, const auto rhs)
        {
            auto lhsEntity = m_EntityIDMap.find(lhs.ID);
            auto rhsEntity = m_EntityIDMap.find(rhs.ID);

            if (lhsEntity == m_EntityIDMap.end() || rhsEntity == m_EntityIDMap.end())
            {
                return lhsEntity != m_EntityIDMap.end();
            }

            return static_cast<uint32_t>(lhsEntity->second) < static_cast<uint32_t>(rhsEntity->second);
        });
    }

    void Scene::DestroyEntity(Entity entity)
    {
        entity.Destroy();
        HandleModified();
    }

    void Scene::OnUpdateEditor(TimeStep ts)
    {
    }

    void Scene::OnUpdateRuntime(TimeStep ts)
    {
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
    }

    void Scene::OnRenderEditor(Ref<SceneRenderer> renderer, TimeStep ts, EditorCamera &camera)
    {
        // 3D Renderer
        renderer->BeginScene(camera);
        PrepareRender(renderer);
        SubmitMeshes(renderer);
        renderer->EndScene();

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
                Renderer2D::DrawSprite(transform.GetTransform(), sprite, (uint32_t)entity);
                // Renderer2D::DrawRect(transform.GetTransform(), sprite.Color, (uint32_t)entity);
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

    void Scene::OnRenderRuntime(Ref<SceneRenderer> renderer, TimeStep ts)
    {
    }

    void Scene::OnViewportResize(float width, float height)
    {
        bool inValid = width <= 0.0f || height <= 0.0f;
        if (inValid || (m_ViewportWidth == width && m_ViewportHeight == height))
            return;

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

    void Scene::PrepareRender(Ref<SceneRenderer> renderer)
    {
        // Skylight
        {
            auto group = m_Registry.group<SkyLightComponent>(entt::get<TransformComponent>);

            for (auto entity : group)
            {
                auto [transform, skyLight] = group.get<TransformComponent, SkyLightComponent>(entity);

                if (skyLight.Dynamic)
                {
                    if (!skyLight.SceneEnvironment)
                        Renderer::CreatePreethamSky(skyLight.TurbidityAzimuthInclination.x, skyLight.TurbidityAzimuthInclination.y, skyLight.TurbidityAzimuthInclination.z);

                    Ref<TextureCube> radiance = Renderer::GetPreethamSkyTextureCube();
                    skyLight.SceneEnvironment = Ref<Environment>::Create(radiance, radiance);
                } else if (skyLight.Source != 0)
                {
                    if (!skyLight.SceneEnvironment)
                    {
                        Ref<Texture2D> texture = Application::GetAssetManager()->GetAsset(skyLight.Source);
                        if (texture)
                            Renderer::CreateStaticSky(texture);
                    }
        
                    Ref<TextureCube> radiance = Renderer::GetStaticSkyTextureCube();
                    Ref<TextureCube> irradiance = Renderer::GetIrradianceTextureCube();
                    skyLight.SceneEnvironment = Ref<Environment>::Create(radiance, irradiance);
                }
                m_Environment = skyLight.SceneEnvironment;
                m_EnvironmentIntensity = skyLight.Intensity;
                m_SkyboxLod = skyLight.Lod;
            }
        }

        // Process lights
        {
            // Directional light
            {
                auto view = m_Registry.view<TransformComponent, DirectionalLightComponent>();
                for (auto entity : view)
                {
                    if (!m_Registry.valid(entity))
                        continue;

                    const auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
                    renderer->SubmitDirectionalLight(&light);
                }
            }
            // Point lights
            {
                auto view = m_Registry.view<TransformComponent, PointLightComponent>();
                for (auto entity : view)
                {
                    if (!m_Registry.valid(entity))
                        continue;

                    const auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
                    renderer->SubmitPointLight(&light, transform.Translation);
                }
            }
            // Spot lights
            {
                auto view = m_Registry.view<TransformComponent, SpotLightComponent>();
                for (auto entity : view)
                {
                    if (!m_Registry.valid(entity))
                        continue;

                    const auto [transform, light] = view.get<TransformComponent, SpotLightComponent>(entity);
                    renderer->SubmitSpotLight(&light, transform.Translation);
                }
            }
        }
    }

    void Scene::SubmitMeshes(Ref<SceneRenderer> renderer)
    {
        auto view = m_Registry.view<TransformComponent, MeshComponent>();
        for (auto entity : view)
        {
            if (!m_Registry.valid(entity))
                continue;

            const auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

            if (mesh.Type == MeshType::Dynamic)
            {
                Entity e = Entity(entity, this);
                Ref<DynamicMesh> dynamicMesh = mesh.MeshInstance.As<DynamicMesh>();
                auto material = Application::GetAssetManager()->GetAsset(mesh.MaterialHandle);
                auto transform = GetWorldSpaceTransformMatrix(e);
                
                renderer->SubmitMesh(dynamicMesh, mesh.SubmeshIndex, material, transform, (uint64_t)entity);
            }
            else if (mesh.Type == MeshType::Static)
            {
                // StaticMesh* staticMesh = dynamic_cast<StaticMesh*>(mesh.MeshInstance.get());
                // staticMesh->GetMeshSource()->SetLocalTransform(transform.GetTransform());
                // if (staticMesh->GetMeshSource()->IsBufferChanged())
                //     Renderer::SubmitStaticMesh(staticMesh);
            }
        }
    }

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
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<SkyLightComponent>(Entity entity, SkyLightComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }

    template<>
    void Scene::OnComponentAdded<SpotLightComponent>(Entity entity, SpotLightComponent& component)
    {
        component.RegisterGlobalCallback([this]() {
            HandleModified();
        });
    }
}
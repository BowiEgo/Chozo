#pragma once

#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/Mesh.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/Renderer/Geometry/Geometry.h"
#include "Chozo/Renderer/Environment.h"
#include "Chozo/Core/UUID.h"
#include "SceneCamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Chozo {

    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {}
    };

    struct TransformComponent
    {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation)
            : Translation(translation) {}

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
            
            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };
    
    struct SpriteRendererComponent
    {
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
        Ref<Texture2D> Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}
    };

    struct CircleRendererComponent
    {
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
        float Radius = 0.5f, Thickness = 0.1f, Fade = 0.001f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
    };

    struct MeshComponent
    {
        Ref<Mesh> MeshInstance;
        Ref<MeshSource> MeshSrc;
        Ref<Material> MaterialInstance;

        MeshType Type = MeshType::Dynamic;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) { CZ_CORE_WARN("Copy constructor called!"); };
        MeshComponent(MeshComponent&& other) noexcept
            : MeshInstance(std::move(other.MeshInstance)),
              Type(other.Type)
        {
            other.MeshSrc.reset();  // Prevent copying or using original object.
        }
        MeshComponent(Ref<MeshSource> meshSrc, MeshType meshType = MeshType::Dynamic, Ref<Material> material = nullptr)
            : MeshSrc(meshSrc), Type(meshType), MaterialInstance(material)
        {
            if (!MaterialInstance)
                MaterialInstance = Material::Create("Basic");

            GenerateMeshInstance();
        }
        ~MeshComponent() {}

        void GenerateMeshInstance()
        {
            MeshSrc->CallGenerate();
            MeshInstance.reset();

            switch (Type) {
                case MeshType::Dynamic:
                    MeshInstance = std::make_shared<DynamicMesh>(MeshSrc);
                    break;
                case MeshType::Instanced:
                    MeshInstance = std::make_shared<InstancedMesh>(MeshSrc);
                    break;
                case MeshType::Static:
                    MeshInstance = std::make_shared<StaticMesh>(MeshSrc);
                    break;
                default:
                    CZ_CORE_ERROR("Unknown mesh type!");
                    break;
            }

            MeshSrc->SetBufferChanged(true);
        }
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool Primary = true; // TODO: think about moving to scene
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    class ScriptableEntity;
    struct NativeScriptComponent
    {
        Ref<ScriptableEntity> Instance;

        Ref<ScriptableEntity>(*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);

        template<typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { nsc->Instance.reset(); };
        }
    };

    struct SkyLightComponent
    {
        Ref<Environment> SceneEnvironment;

        float Intensity = 1.0f;
        float Lod = 0.0f;

        bool DynamicSky = true;
        glm::vec3 TurbidityAzimuthInclination = { 2.74f, 0.0f, 0.0f };
    };
}

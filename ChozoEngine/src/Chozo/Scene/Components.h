#pragma once

#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/Mesh.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/Renderer/Geometry/Geometry.h"
#include "Chozo/Renderer/Environment.h"
#include "Chozo/Core/UUID.h"
#include "Chozo/Math/Math.h"
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

    struct RelationshipComponent
	{
		UUID ParentHandle = 0;
		std::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent& other) = default;
		RelationshipComponent(UUID parent)
			: ParentHandle(parent) {}
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

		void SetTransform(const glm::mat4& transform)
		{
			Math::DecomposeTransform(transform, Translation, Rotation, Scale);
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
		uint32_t SubmeshIndex = 0;
        AssetHandle MaterialHandle;

        MeshType Type = MeshType::Dynamic;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) { CZ_CORE_WARN("Copy constructor called!"); };
        MeshComponent(MeshComponent&& other) noexcept
            : MeshInstance(std::move(other.MeshInstance)),
              Type(other.Type)
        {}
        MeshComponent(Ref<Mesh> mesh, uint32_t submeshIndex = 0, AssetHandle materialHandle = 0)
            : MeshInstance(mesh), SubmeshIndex(submeshIndex)
        {
            if (materialHandle != 0)
            {
                SetMaterial(materialHandle);
            }
            else
            {
                auto meshSource = MeshInstance->GetMeshSource();
                auto materialIndex = meshSource->GetSubmeshes()[SubmeshIndex].MaterialIndex;
                auto materials = meshSource->GetMaterials();
                if (!materials.empty())
                    MaterialHandle = materials[materialIndex];
                else
                    MaterialHandle = 0;
            }
        }

        void SetMaterial(AssetHandle materialHandle)
        {
            MaterialHandle = materialHandle;
            MeshInstance->SetMaterial(SubmeshIndex, materialHandle);
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
            DestroyScript = [](NativeScriptComponent* nsc) { nsc->Instance.Reset(); };
        }
    };

    struct SkyLightComponent
    {
        Ref<Environment> SceneEnvironment;

        float Intensity = 1.0f;
        float Lod = 0.0f;

        bool Dynamic = true;
        AssetHandle Source;
        glm::vec3 TurbidityAzimuthInclination = { 2.74f, 0.0f, 0.0f };
    };

    enum class LightType
	{
		None = 0, Directional = 1, Point = 2, Spot = 3
	};

    struct DirectionalLightComponent
	{
		glm::vec3 Direction = { -45.0f, 45.0f, 45.0f };
		glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
	};

    struct PointLightComponent
	{
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };;
        float Intensity = 1.0f;
	};

    struct SpotLightComponent
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float Intensity = 1.0f;
		glm::vec3 Direction = { 0.0f, 0.0f, -1.0f };
		float AngleAttenuation = 5.0f;
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };;
		float Angle = 10.0f;
	};
}

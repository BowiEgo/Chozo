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

    struct Component
    {
        using GlobalCallback = std::function<void()>;
        using Callback = std::function<void(const std::string& propertyName)>;

        virtual ~Component() = default;

        void RegisterGlobalCallback(const GlobalCallback &callback)
        {
            globalCallbacks.push_back(callback);
        }

        void RegisterCallback(const std::string& propertyName, const Callback &callback)
        {
            callbacks[propertyName].push_back(callback);
        }

        void NotifyChange(const std::string& propertyName)
        {
            for (const auto& callback : globalCallbacks)
            {
                if (callback)
                    callback();
            }

            if (const auto it = callbacks.find(propertyName); it != callbacks.end())
            {
                for (const auto& callback : it->second)
                {
                    if (callback)
                        callback(propertyName);
                }
            }
        }
    private:
        std::vector<GlobalCallback> globalCallbacks;
        std::unordered_map<std::string, std::vector<Callback>> callbacks;
    };

    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent : Component
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {}

        void SetTag(const std::string& tag) { Tag = tag; NotifyChange("Tag"); }
    };

    struct RelationshipComponent : Component
	{
		UUID ParentHandle = 0;
		std::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent& other) = default;
		RelationshipComponent(UUID parent)
			: ParentHandle(parent) {}

        void SetParent(const UUID parent) { ParentHandle = parent; NotifyChange("ParentHandle"); }
        void SetChildren(const std::vector<UUID>& children) { Children = children; NotifyChange("Children"); }
	};

    struct TransformComponent : Component
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
            NotifyChange("Transform");
		}

        void SetTranslation(const glm::vec3& translation) { Translation = translation; NotifyChange("Translation"); }
        void SetRotation(const glm::vec3& rotation) { Rotation = rotation; NotifyChange("Rotation"); }
        void SetScale(const glm::vec3& scale) { Scale = scale; NotifyChange("Scale"); }
    };
    
    struct SpriteRendererComponent : Component
    {
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
        Ref<Texture2D> Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}

        void SetColor(const glm::vec4& color) { Color = color; NotifyChange("Color"); }
        void SetTexture(const Ref<Texture2D>& texture) { Texture = texture; NotifyChange("Texture"); }
        void SetTilingFactor(const float factor) { TilingFactor = factor; NotifyChange("TilingFactor"); }
    };

    struct CircleRendererComponent : Component
    {
        glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
        float Radius = 0.5f, Thickness = 0.1f, Fade = 0.001f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;

        void SetColor(const glm::vec4& color) { Color = color; NotifyChange("Color"); }
        void SetRadius(const float radius) { Radius = radius; NotifyChange("Radius"); }
        void SetThickness(const float thickness) { Thickness = thickness; NotifyChange("Thickness"); }
        void SetFade(const float fade) { Fade = fade; NotifyChange("Fade"); }
    };

    struct MeshComponent : Component
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

            MeshInstance->RegisterOnChange([this]() {
                NotifyChange("MeshInstance");
            });
        }

        void SetMaterial(AssetHandle materialHandle)
        {
            MaterialHandle = materialHandle;
            MeshInstance->SetMaterial(SubmeshIndex, materialHandle);
            NotifyChange("Material");
        }
    };

    struct CameraComponent : Component
    {
        SceneCamera Camera;
        bool Primary = true; // TODO: think about moving to scene
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    class ScriptableEntity;
    struct NativeScriptComponent : Component
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

    struct SkyLightComponent : Component
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

    struct DirectionalLightComponent : Component
	{
		glm::vec3 Direction = { -45.0f, 45.0f, 45.0f };
		glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;

        void SetDirection(const glm::vec3& direction) { Direction = direction; NotifyChange("Direction"); }
        void SetColor(const glm::vec3& color) { Color = color; NotifyChange("Color"); }
        void SetIntensity(const float intensity) { Intensity = intensity; NotifyChange("Intensity"); }
	};

    struct PointLightComponent : Component
	{
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };;
        float Intensity = 1.0f;
	};

    struct SpotLightComponent : Component
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float Intensity = 1.0f;
		glm::vec3 Direction = { 0.0f, 0.0f, -1.0f };
		float AngleAttenuation = 5.0f;
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };;
		float Angle = 10.0f;
	};
}

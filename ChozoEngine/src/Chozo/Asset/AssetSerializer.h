#pragma once

#include "Asset.h"
#include "Chozo/Scene/Scene.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/FileSystem/FileStream.h"

namespace Chozo {

	struct AssetFileHeader
	{
		uint32_t Version = 1;
		uint16_t Type{};
	};

    class AssetSerializer
    {
    public:
		virtual ~AssetSerializer() = default;

		virtual uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
		virtual Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const = 0;
    };

	//==============================================================================
	// SceneSerializer

	class SceneSerializer final : public AssetSerializer
	{
	public:
		uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	private:
		static std::string SerializeToYAML(Ref<Scene> scene) ;
		static Ref<Scene> DeserializeFromYAML(const std::string& yamlString) ;
	};

	//==============================================================================
	// TextureSerializer
	struct TextureFileMetadata
	{
		uint16_t Format;

		uint32_t Samples;
		uint32_t Width, Height;
		uint8_t Mipmap;
		uint16_t WrapR;
		uint16_t WrapS;
		uint16_t WrapT;
		uint16_t MinFilter;
		uint16_t MagFilter;
	};

    class TextureSerializer final : public AssetSerializer
	{
	public:
		uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	};

	//==============================================================================
	// MaterialSerializer
	class MaterialSerializer final : public AssetSerializer
	{
	public:
		uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	private:
		static std::string SerializeToYAML(Ref<Material> material) ;
		static Ref<Material> DeserializeFromYAML(const std::string& yamlString) ;
	};

	//==============================================================================
	// MeshSourceSerializer
	struct MeshMaterial
	{
		std::string Name;
		std::string ShaderName;

		glm::vec3 BaseColor;
		float Metallic;
		float Roughness;
		float Reflectance;
		float Ambient;
		float AmbientStrength;

		bool EnableBaseColorTex;
		bool EnableNormalTex;
		bool EnableMetallicTex;
		bool EnableRoughnessTex;

		AssetHandle BaseColorTexture;
		AssetHandle NormalTexture;
		AssetHandle MetallicTexture;
		AssetHandle RoughnessTexture;

		static void Serialize(StreamWriter* serializer, const MeshMaterial& instance)
		{
			serializer->WriteString(instance.Name);
			serializer->WriteString(instance.ShaderName);

			serializer->WriteRaw(instance.BaseColor);
			serializer->WriteRaw(instance.Metallic);
			serializer->WriteRaw(instance.Roughness);
			serializer->WriteRaw(instance.Reflectance);
			serializer->WriteRaw(instance.Ambient);
			serializer->WriteRaw(instance.AmbientStrength);

			serializer->WriteRaw(instance.EnableBaseColorTex);
			serializer->WriteRaw(instance.EnableNormalTex);
			serializer->WriteRaw(instance.EnableMetallicTex);
			serializer->WriteRaw(instance.EnableRoughnessTex);
			
			serializer->WriteRaw(instance.BaseColorTexture);
			serializer->WriteRaw(instance.NormalTexture);
			serializer->WriteRaw(instance.MetallicTexture);
			serializer->WriteRaw(instance.RoughnessTexture);
		}

		static void Deserialize(StreamReader* deserializer, MeshMaterial& instance)
		{
			deserializer->ReadString(instance.Name);
			deserializer->ReadString(instance.ShaderName);

			deserializer->ReadRaw(instance.BaseColor);
			deserializer->ReadRaw(instance.Metallic);
			deserializer->ReadRaw(instance.Roughness);
			deserializer->ReadRaw(instance.Reflectance);
			deserializer->ReadRaw(instance.Ambient);
			deserializer->ReadRaw(instance.AmbientStrength);

			deserializer->ReadRaw(instance.EnableBaseColorTex);
			deserializer->ReadRaw(instance.EnableNormalTex);
			deserializer->ReadRaw(instance.EnableMetallicTex);
			deserializer->ReadRaw(instance.EnableRoughnessTex);
			
			deserializer->ReadRaw(instance.BaseColorTexture);
			deserializer->ReadRaw(instance.NormalTexture);
			deserializer->ReadRaw(instance.MetallicTexture);
			deserializer->ReadRaw(instance.RoughnessTexture);
		}
	};
	
	struct MeshSourceFileMetadata
	{
		enum class MeshFlags : uint32_t
		{
			HasMaterials = BIT(0),
			HasAnimation = BIT(1),
			HasSkeleton = BIT(2)
		};

		uint32_t Flags{};
		AABB BoundingBox;

		uint64_t NodeArrayOffset{};
		uint64_t NodeArraySize{};

		uint64_t SubMeshArrayOffset{};
		uint64_t SubMeshArraySize{};

		uint64_t VertexBufferOffset{};
		uint64_t VertexBufferSize{};

		uint64_t IndexBufferOffset{};
		uint64_t IndexBufferSize{};

		uint64_t MaterialArrayOffset{};
		uint64_t MaterialArraySize{};
	};

	class MeshSourceSerializer final : public AssetSerializer
	{
	public:
		uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	private:
		static std::string SerializeToYAML(const Ref<MeshSource>& meshSource) ;
		static Ref<MeshSource> DeserializeFromYAML(const std::string& yamlString) ;
	};
}

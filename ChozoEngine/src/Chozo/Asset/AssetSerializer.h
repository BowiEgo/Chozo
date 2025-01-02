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
	// struct MeshMaterial
	// {
	// 	std::string Name;
	// 	std::string ShaderName;
	//
	// 	glm::vec3 BaseColor;
	// 	float OcclusionIntensity;
	// 	glm::vec3 Emissive;
	// 	float EmissiveIntensity;
	// 	float Metallic;
	// 	float Roughness;
	//
	// 	bool EnableBaseColorMap;
	// 	bool EnableMetallicRoughnessMap;
	// 	bool EnableNormalMap;
	// 	bool EnableOcclusionMap;
	// 	bool EnableEmissiveMap;
	//
	// 	static void Serialize(StreamWriter* serializer, const MeshMaterial& instance)
	// 	{
	// 		serializer->WriteString(instance.Name);
	// 		serializer->WriteString(instance.ShaderName);
	//
	// 		serializer->WriteRaw(instance.BaseColor);
	// 		serializer->WriteRaw(instance.OcclusionIntensity);
	// 		serializer->WriteRaw(instance.Emissive);
	// 		serializer->WriteRaw(instance.EmissiveIntensity);
	// 		serializer->WriteRaw(instance.Metallic);
	// 		serializer->WriteRaw(instance.Roughness);
	//
	// 		serializer->WriteRaw(instance.EnableBaseColorMap);
	// 		serializer->WriteRaw(instance.EnableMetallicRoughnessMap);
	// 		serializer->WriteRaw(instance.EnableNormalMap);
	// 		serializer->WriteRaw(instance.EnableOcclusionMap);
	// 		serializer->WriteRaw(instance.EnableEmissiveMap);
	// 	}
	//
	// 	static void Deserialize(StreamReader* deserializer, MeshMaterial& instance)
	// 	{
	// 		deserializer->ReadString(instance.Name);
	// 		deserializer->ReadString(instance.ShaderName);
	//
	// 		deserializer->ReadRaw(instance.BaseColor);
	// 		deserializer->ReadRaw(instance.OcclusionIntensity);
	// 		deserializer->ReadRaw(instance.Emissive);
	// 		deserializer->ReadRaw(instance.EmissiveIntensity);
	// 		deserializer->ReadRaw(instance.Metallic);
	// 		deserializer->ReadRaw(instance.Roughness);
	//
	// 		deserializer->ReadRaw(instance.EnableBaseColorMap);
	// 		deserializer->ReadRaw(instance.EnableMetallicRoughnessMap);
	// 		deserializer->ReadRaw(instance.EnableNormalMap);
	// 		deserializer->ReadRaw(instance.EnableOcclusionMap);
	// 		deserializer->ReadRaw(instance.EnableEmissiveMap);
	// 	}
	// };
	//
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

		// uint64_t MaterialArrayOffset{};
		// uint64_t MaterialArraySize{};
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

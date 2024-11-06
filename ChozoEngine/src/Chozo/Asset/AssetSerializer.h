#pragma once

#include "Asset.h"
#include "Chozo/Scene/Scene.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/FileSystem/FileStream.h"

namespace Chozo {

	struct AssetFileHeader
	{
		uint32_t Version = 1;
		uint16_t Type;
		uint64_t CreateAt;
		uint64_t ModifiedAt;
	};

    class AssetSerializer
    {
    public:
		virtual ~AssetSerializer() = default;

		virtual uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
		virtual Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const = 0;
    };

	//==============================================================================
	/// SceneSerializer

	class SceneSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	private:
		std::string SerializeToYAML(Ref<Scene> scene) const;
		Ref<Scene> DeserializeFromYAML(const std::string& yamlString) const;
	};

	//==============================================================================
	/// TextureSerializer
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

    class TextureSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	};

	//==============================================================================
	/// MaterialSerializer
	class MaterialSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	private:
		std::string SerializeToYAML(Ref<Material> material) const;
		Ref<Material> DeserializeFromYAML(const std::string& yamlString) const;
	};

	//==============================================================================
	/// MeshSourceSerializer
	struct MeshMaterial
	{
		std::string Name;
		std::string ShaderName;

		glm::vec3 Albedo;
		float Metalness;
		float Roughness;
		float Ambient;
		float AmbientStrength;
		float Specular;

		bool EnableAlbedoTex;
		bool EnableNormalTex;
		bool EnableMetalnessTex;
		bool EnableRoughnessTex;

		AssetHandle AlbedoTexture;
		AssetHandle NormalTexture;
		AssetHandle MetalnessTexture;
		AssetHandle RoughnessTexture;

		static void Serialize(StreamWriter* serializer, const MeshMaterial& instance)
		{
			serializer->WriteString(instance.Name);
			serializer->WriteString(instance.ShaderName);

			serializer->WriteRaw(instance.Albedo);
			serializer->WriteRaw(instance.Metalness);
			serializer->WriteRaw(instance.Roughness);
			serializer->WriteRaw(instance.Ambient);
			serializer->WriteRaw(instance.AmbientStrength);
			serializer->WriteRaw(instance.Specular);

			serializer->WriteRaw(instance.EnableAlbedoTex);
			serializer->WriteRaw(instance.EnableNormalTex);
			serializer->WriteRaw(instance.EnableMetalnessTex);
			serializer->WriteRaw(instance.EnableRoughnessTex);
			
			serializer->WriteRaw(instance.AlbedoTexture);
			serializer->WriteRaw(instance.NormalTexture);
			serializer->WriteRaw(instance.MetalnessTexture);
			serializer->WriteRaw(instance.RoughnessTexture);
		}

		static void Deserialize(StreamReader* deserializer, MeshMaterial& instance)
		{
			deserializer->ReadString(instance.Name);
			deserializer->ReadString(instance.ShaderName);

			deserializer->ReadRaw(instance.Albedo);
			deserializer->ReadRaw(instance.Metalness);
			deserializer->ReadRaw(instance.Roughness);
			deserializer->ReadRaw(instance.Ambient);
			deserializer->ReadRaw(instance.AmbientStrength);
			deserializer->ReadRaw(instance.Specular);

			deserializer->ReadRaw(instance.EnableAlbedoTex);
			deserializer->ReadRaw(instance.EnableNormalTex);
			deserializer->ReadRaw(instance.EnableMetalnessTex);
			deserializer->ReadRaw(instance.EnableRoughnessTex);
			
			deserializer->ReadRaw(instance.AlbedoTexture);
			deserializer->ReadRaw(instance.NormalTexture);
			deserializer->ReadRaw(instance.MetalnessTexture);
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

		uint32_t Flags;
		AABB BoudingBox;

		uint64_t NodeArrayOffset;
		uint64_t NodeArraySize;

		uint64_t SubmeshArrayOffset;
		uint64_t SubmeshArraySize;

		uint64_t VertexBufferOffset;
		uint64_t VertexBufferSize;

		uint64_t IndexBufferOffset;
		uint64_t IndexBufferSize;

		uint64_t MaterialArrayOffset;
		uint64_t MaterialArraySize;
	};

	class MeshSourceSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(FileStreamWriter& stream, const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(FileStreamReader& stream, AssetMetadata& metadata) const override;
	private:
		std::string SerializeToYAML(Ref<MeshSource> meshSource) const;
		Ref<MeshSource> DeserializeFromYAML(const std::string& yamlString) const;
	};
}

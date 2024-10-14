#include "AssetSerializer.h"

#include "Chozo/Core/Application.h"
#include "Chozo/Project/Project.h"
#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/Material.h"

#include "Chozo/FileSystem/FileStream.h"
#include "Chozo/Utilities/SerializationUtils.h"

#include <yaml-cpp/yaml.h>

namespace Chozo {

    extern const fs::path g_AssetsPath;

	//==============================================================================
	/// TextureSerializer
    uint64_t TextureSerializer::Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = g_AssetsPath / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);

		Buffer buffer;
        Ref<Texture2D> texture = asset.As<Texture2D>();
        texture->CopyToHostBuffer(buffer);
        
		Texture2DMetadata texture2DMetadata;
        texture2DMetadata.Width = texture->GetWidth();
        texture2DMetadata.Height = texture->GetHeight();
        texture2DMetadata.Format = (uint16_t)texture->GetSpecification().Format;
		stream.WriteRaw(texture2DMetadata);
		stream.WriteBuffer(buffer);
        uint64_t size = buffer.GetSize() + sizeof(texture2DMetadata);

        buffer.Release();

        return size;
    }

    Ref<Asset> TextureSerializer::Deserialize(const AssetMetadata& metadata) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = g_AssetsPath / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamReader stream(dest);
		Texture2DMetadata texture2DMetadata;
		stream.ReadRaw<Texture2DMetadata>(texture2DMetadata);

        Buffer buffer;
        stream.ReadBuffer(buffer);

        Texture2DSpecification spec;
		spec.Width = texture2DMetadata.Width;
		spec.Height = texture2DMetadata.Height;
		spec.Format = (ImageFormat)texture2DMetadata.Format;
		spec.Mipmap = false;
        Ref<Texture2D> texture = Texture2D::Create(buffer, spec);

        buffer.Release();

        return texture;
    }

    //==============================================================================
	/// MaterialSerializer
    uint64_t MaterialSerializer::Serialize(const AssetMetadata &metadata, Ref<Asset> &asset) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = g_AssetsPath / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);

        Ref<Material> material = asset.As<Material>();
		std::string yamlString = SerializeToYAML(material);
        
        uint64_t start = stream.GetStreamPosition();
		stream.WriteString(yamlString);
        uint64_t size = stream.GetStreamPosition() - start;

#if 1
        fs::path dest2 = filepath.parent_path() / (filepath.filename().string() + ".txt");
        fs::path dirPath(dest2);
        if (!(fs::exists(dirPath) && fs::is_directory(dirPath)))
            fs::create_directories(dirPath.parent_path().string());

        std::ofstream fout(filepath);
        fout << yamlString.c_str();
#endif
        return size;
    }

    Ref<Asset> MaterialSerializer::Deserialize(const AssetMetadata &metadata) const
    {
        fs::path path(metadata.FilePath);
        fs::path filepath = g_AssetsPath / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamReader stream(dest);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<Material> material = DeserializeFromYAML(yamlString);
        return material;
    }

    std::string MaterialSerializer::SerializeToYAML(Ref<Material> material) const
    {
        YAML::Emitter out;
		out << YAML::BeginMap; // Material
		out << YAML::Key << "Material" << YAML::Value;
		{
			out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << material->GetName();
            for (auto [name, value] : material->GetUniforms())
            {
                if (name.find("u_Material") != std::string::npos)
                    Utils::Serialization::SerializeProperty(name, value, out);
            }
            out << YAML::EndMap;
        }

        {
            out << YAML::Key << "Textures" << YAML::Value;
			out << YAML::BeginMap;
            for (auto [name, handle] : material->GetTextureAssetHandles())
            {
                if (name.find("Tex") != std::string::npos)
                    out << YAML::Key << name << YAML::Value << handle;
            }
            std::vector<AssetHandle> textureHandles;
            out << YAML::EndMap;
        }

		out << YAML::EndMap; // Material
		return std::string(out.c_str());
    }

    Ref<Material> MaterialSerializer::DeserializeFromYAML(const std::string &yamlString) const
    {
        YAML::Node root = YAML::Load(yamlString);
		YAML::Node materialNode = root["Material"];
		YAML::Node texturesNode = root["Textures"];

        Ref<Material> material = Material::Create(materialNode["Name"].as<std::string>());
        for (const auto& pair : materialNode)
        {
            std::string key = pair.first.as<std::string>();
            if (key == "Name")
                continue;

            const YAML::Node& valueNode = pair.second;
            UniformValue value;

            if (valueNode.IsScalar()) {
                try {
                    try {
                        value = valueNode.as<bool>();
                    } catch (const YAML::BadConversion& e) {
                    }
                    try {
                        value = valueNode.as<int>();
                    } catch (const YAML::BadConversion& e) {
                    }
                    try {
                        value = valueNode.as<float>();
                    } catch (const YAML::BadConversion& e) {
                    }
                } catch (const YAML::Exception& e) {
                    CZ_CORE_ERROR("Unknown type: {0}", e.what());
                }
            } else if (valueNode.IsSequence()) {
                if (valueNode.size() == 2) {
                    value = glm::vec2(valueNode[0].as<float>(), valueNode[1].as<float>());
                } else if (valueNode.size() == 3) {
                    value = glm::vec3(valueNode[0].as<float>(), valueNode[1].as<float>(), valueNode[2].as<float>());
                } else if (valueNode.size() == 4) {
                    value = glm::vec4(valueNode[0].as<float>(), valueNode[1].as<float>(), valueNode[2].as<float>(), valueNode[3].as<float>());
                }
            }

            material->Set(key, value);
        }

        for (const auto& pair : texturesNode)
        {
            std::string key = pair.first.as<std::string>();
            AssetHandle handle = pair.second.as<std::uint64_t>();
            material->SetTextureHandle(key, handle);
        }

        return material;
    }
}

#include "AssetImporter.h"

namespace Chozo
{

	std::unordered_map<AssetType, Scope<AssetSerializer>> AssetImporter::s_Serializers;

    void AssetImporter::Init()
    {
		s_Serializers.clear();
        for (int i = 0; i < static_cast<int>(AssetType::None); i++)
        {
	        switch (auto type = static_cast<AssetType>(i))
            {
				case AssetType::Texture:
					s_Serializers[type] = CreateScope<TextureSerializer>(); break;
				case AssetType::Material:
					s_Serializers[type] = CreateScope<MaterialSerializer>(); break;
				case AssetType::Scene:
					s_Serializers[type] = CreateScope<SceneSerializer>(); break;
        		case AssetType::MeshSource:
					s_Serializers[type] = CreateScope<MeshSourceSerializer>(); break;
				default:
					break;
            }
        }
    }

    uint64_t AssetImporter::Serialize(const AssetMetadata& metadata, Ref<Asset>& asset)
    {
		fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        fs::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        FileStreamWriter stream(dest);
        // uint64_t start = stream.GetStreamPosition();
        AssetFileHeader header;

        // Write header
        header.Type = static_cast<uint16_t>(metadata.Type);
        header.CreateAt = metadata.CreateAt;
        header.ModifiedAt = metadata.ModifiedAt;
		stream.WriteRaw<AssetFileHeader>(header);

		return s_Serializers[metadata.Type]->Serialize(stream, metadata, asset);
    }

    Ref<Asset> AssetImporter::Deserialize(AssetMetadata &metadata)
    {
		fs::path dest;
		fs::path path(metadata.FilePath);
        fs::path filepath = Utils::File::GetAssetDirectory() / path;
        dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

        if (!fs::exists(dest))
        {
            CZ_CORE_ERROR("Asset file doesn't exist at {}", dest.string());
            return nullptr;
        }

        FileStreamReader stream(dest);
		AssetFileHeader header;

		// Read header
		stream.ReadRaw<AssetFileHeader>(header);
        metadata.CreateAt = header.CreateAt;
        metadata.ModifiedAt = header.ModifiedAt;
		metadata.Type = static_cast<AssetType>(header.Type);
		
		return s_Serializers[metadata.Type]->Deserialize(stream, metadata);
    }
}

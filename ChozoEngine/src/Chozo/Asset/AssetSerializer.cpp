#include "AssetSerializer.h"

#include "Chozo/Project/Project.h"
#include "Chozo/Renderer/Texture.h"

#include "Chozo/FileSystem/FileStream.h"

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;

	//==============================================================================
	/// TextureSerializer
    uint64_t TextureSerializer::Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const
    {
        std::filesystem::path path(metadata.FilePath);
        std::filesystem::path filepath = g_AssetsPath / path;
        std::filesystem::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

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
        uint64_t size = buffer.GetSize();

        buffer.Release();

        return size;
    }

    Ref<Asset> TextureSerializer::Deserialize(const AssetMetadata& metadata) const
    {
        std::filesystem::path path(metadata.FilePath);
        std::filesystem::path filepath = g_AssetsPath / path;
        std::filesystem::path dest = filepath.parent_path() / (filepath.filename().string() + ".asset");

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
}

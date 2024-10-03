#include "ThumbnailManager.h"
#include "ThumbnailExporter.h"

namespace Chozo {

    ThumbnailManager::ThumbnailManager()
    {
        ThumbnailExporter::Init();
    }

    ThumbnailManager::~ThumbnailManager()
    {
    }

    void ThumbnailManager::ImportThumbnail(AssetHandle assetHandle)
    {
        std::string filename = std::to_string(assetHandle) + ".png";
        fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        fs::path filepath = cacheDir / filename;

        Texture2DSpecification spec;
        spec.Format = ImageFormat::RGBA;
        Ref<Texture2D> thumbnail = Texture2D::Create(filepath.string(), spec);

        m_Thumbnails[assetHandle] = thumbnail;
    }

    void ThumbnailManager::CreateThumbnail(AssetMetadata metadata, Ref<Asset> asset)
    {
        Ref<Texture2D> thumbnail = ThumbnailExporter::Render(metadata, asset);
        m_Thumbnails[metadata.Handle] = thumbnail;
    }

    void ThumbnailManager::RemoveThumbnail(AssetHandle assetHandle)
    {
        if (m_Thumbnails.find(assetHandle) != m_Thumbnails.end())
        {
            std::string filename = std::to_string(assetHandle) + ".png";
            fs::path filePath = Utils::File::GetThumbnailCacheDirectory() / fs::path(filename);
            Utils::File::DeleteFile(filePath);
            m_Thumbnails.erase(assetHandle);
        }
    }

    Ref<Texture2D> ThumbnailManager::GetThumbnail(AssetHandle assetHandle)
    {
        if (m_Thumbnails.find(assetHandle) == m_Thumbnails.end())
            ImportThumbnail(assetHandle);

        return m_Thumbnails[assetHandle];
        // TODO: return errorTexture
    }

} // namespace Chozo

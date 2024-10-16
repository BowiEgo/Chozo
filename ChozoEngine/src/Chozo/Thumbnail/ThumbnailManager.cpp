#include "ThumbnailManager.h"

namespace Chozo {

    Ref<ThumbnailManager> ThumbnailManager::s_Instance;

    void ThumbnailManager::Init()
    {
        s_Instance = Ref<ThumbnailManager>::Create();
    }

    void ThumbnailManager::ImportThumbnail(AssetHandle handle)
    {
        std::string filename = std::to_string(handle) + ".png";
        fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        fs::path filepath = cacheDir / filename;

        Texture2DSpecification spec;
        spec.Format = ImageFormat::RGBA;
        Ref<Texture2D> thumbnail = Texture2D::Create(filepath.string(), spec);

        SetThumbnail(handle, thumbnail);
    }

    void ThumbnailManager::DeleteThumbnail(AssetHandle assetHandle)
    {
        auto thumbnails = s_Instance->m_Thumbnails;

        if (thumbnails.find(assetHandle) != thumbnails.end())
        {
            std::string filename = std::to_string(assetHandle) + ".png";
            fs::path filePath = Utils::File::GetThumbnailCacheDirectory() / fs::path(filename);
            Utils::File::DeleteFile(filePath);
            thumbnails.erase(assetHandle);
        }
    }

    Ref<Texture2D> ThumbnailManager::GetThumbnail(AssetHandle assetHandle)
    {
        auto thumbnails = s_Instance->m_Thumbnails;

        if (thumbnails.find(assetHandle) == thumbnails.end())
            ImportThumbnail(assetHandle);

        if (thumbnails[assetHandle])
            return thumbnails[assetHandle];
        else
            return Texture2D::Create(); // TODO: return errorTexture
    }

} // namespace Chozo

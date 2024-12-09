#include "ThumbnailManager.h"

#include "Chozo/Core/Application.h"

namespace Chozo {

    ThumbnailManager* ThumbnailManager::s_Instance;

    void ThumbnailManager::Init()
    {
        s_Instance = new ThumbnailManager();
        ClearUselessCaches();
    }

    void ThumbnailManager::Shutdown()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }

    void ThumbnailManager::ImportThumbnail(AssetHandle handle)
    {
        Ref<Texture2D> thumbnail;

        std::string filename = std::to_string(handle) + ".png";
        const fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        const fs::path filepath = cacheDir / filename;
        if (fs::exists(filepath))
        {
            Texture2DSpecification spec;
            spec.Format = ImageFormat::RGBA;
            thumbnail = Texture2D::Create(filepath.string(), spec);
        }
        else
            thumbnail = Renderer::GetCheckerboardTexture();

        SetThumbnail(handle, thumbnail);
    }

    void ThumbnailManager::DeleteThumbnail(AssetHandle assetHandle)
    {
        if (auto thumbnails = s_Instance->m_Thumbnails; thumbnails.find(assetHandle) != thumbnails.end())
        {
            const std::string filename = std::to_string(assetHandle) + ".png";
            const fs::path filePath = Utils::File::GetThumbnailCacheDirectory() / fs::path(filename);
            Utils::File::DeleteFile(filePath);
            thumbnails.erase(assetHandle);
        }
    }

    void ThumbnailManager::ClearUselessCaches()
    {
        const fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());

		for (const auto& entry : fs::directory_iterator(cacheDir))
        {
            if (!entry.is_directory())
            {
                const std::string fileName = entry.path().stem().string();

                if (AssetHandle handle = Utils::String::ToUint64(fileName); handle != 0)
                {
                    if (const bool exist = Application::GetAssetManager()->IsAssetHandleValid(handle); !exist)
                        Utils::File::DeleteFile(entry.path());
                }
            }
        }
    }

    Ref<Texture2D> ThumbnailManager::GetThumbnail(const AssetHandle assetHandle)
    {
        auto& thumbnails = s_Instance->m_Thumbnails;

        if (thumbnails.find(assetHandle) == thumbnails.end())
            ImportThumbnail(assetHandle);

        if (thumbnails[assetHandle])
            return thumbnails[assetHandle];
        else
            return nullptr; // TODO: return errorTexture
    }

} // namespace Chozo

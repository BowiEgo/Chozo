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
        fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
        fs::path filepath = cacheDir / filename;
        auto exist = fs::exists(filepath);
        if (exist)
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
        auto thumbnails = s_Instance->m_Thumbnails;

        if (thumbnails.find(assetHandle) != thumbnails.end())
        {
            std::string filename = std::to_string(assetHandle) + ".png";
            fs::path filePath = Utils::File::GetThumbnailCacheDirectory() / fs::path(filename);
            Utils::File::DeleteFile(filePath);
            thumbnails.erase(assetHandle);
        }
    }

    void ThumbnailManager::ClearUselessCaches()
    {
        fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());

		for (auto entry : fs::directory_iterator(cacheDir))
        {
            if (!entry.is_directory())
            {
                std::string fileName = entry.path().stem().string();
                AssetHandle handle = Utils::String::ToUint64(fileName);

                if (handle != 0)
                {
                    bool exist = Application::GetAssetManager()->IsAssetHandleValid(handle);
                    if (!exist)
                        Utils::File::DeleteFile(entry.path());
                }
            }
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
            return nullptr; // TODO: return errorTexture
    }

} // namespace Chozo

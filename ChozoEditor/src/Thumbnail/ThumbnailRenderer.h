#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Scene/Scene.h"
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/SceneRenderer.h"
#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/FileSystem/TextureExporter.h"

#include "ThumbnailPoolTask.h"

namespace Chozo {

    namespace Utils {

        static void ExportPNG(std::string filename, Buffer source, glm::vec2 srcSize, glm::vec2 outputSize, bool isHDR)
        {
            fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
            fs::path filepath = cacheDir / filename;

            Utils::File::CreateDirectoryIfNeeded(cacheDir.string());

            Buffer buffer = TextureExporter::ToFileFromBuffer(
                            filepath,
                            source,
                            srcSize.x, srcSize.y,
                            outputSize.x, outputSize.y,
                            isHDR);
        }
    }

    class ThumbnailRenderer
    {
    public:
        virtual ~ThumbnailRenderer() = default;
        virtual void Render(Ref<ThumbnailPoolTask> task) = 0;

        static void Init();
        static void RenderTask(Ref<ThumbnailPoolTask> task);

        template <typename T>
		static T* GetRenderer();

        inline static ThumbnailRenderer* GetRenderer(AssetType type) {
            CZ_CORE_ASSERT(s_Renderers[type], std::string(Utils::AssetTypeToString(type)) + "ThumbnailRenderer has not been initialized.");
            return s_Renderers[type].get();
        }
        inline static uint32_t GetThumbnailSize() { return s_ThumbnailSize; }
    private:
        static std::unordered_map<AssetType, Scope<ThumbnailRenderer>> s_Renderers;
        static uint32_t s_ThumbnailSize;
    };

    class TextureThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        virtual void Render(Ref<ThumbnailPoolTask> task) override;
    };

    class MaterialThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        MaterialThumbnailRenderer();
        virtual void Render(Ref<ThumbnailPoolTask> task) override;

        void OnUpdate();
        void RenderToBuffer(SharedBuffer& dest);
    
        Entity GetSphere();
        inline Ref<Material> GetMaterial() { return m_Material; }
        void SetMaterial(Ref<Material> material);
        void SetMaterialValue(Ref<Material> material, std::string name, UniformValue value);
        void SetMaterialValue(Ref<Material> material, std::string name, Ref<Texture2D> texture);
        inline Ref<Scene> GetScene() { return m_Scene; }
        inline Ref<SceneRenderer> GetSceneRenderer() { return m_SceneRenderer; }
        inline Ref<Texture2D> GetOutput() { return m_SceneRenderer->GetCompositePass()->GetOutput(0); }
        inline glm::vec2 GetViewportSize() { return m_ViewportSize; }
    private:
        Ref<Scene> m_Scene;
		Ref<SceneRenderer> m_SceneRenderer;
        Ref<Material> m_Material;
        EditorCamera m_Camera;
        glm::vec2 m_ViewportSize{200, 200};
    };
    
} // namespace Chozo

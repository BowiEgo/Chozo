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

        static void ExportPNG(const std::string& filename, const Buffer &source, const glm::vec2 srcSize, const glm::vec2 outputSize, bool isHDR)
        {
            fs::path cacheDir(Utils::File::GetThumbnailCacheDirectory());
            fs::path filepath = cacheDir / filename;

            File::CreateDirectoryIfNeeded(cacheDir.string());

            TextureExporter::ToFileFromBuffer(
                filepath,
                source,
                static_cast<uint32_t>(srcSize.x), static_cast<uint32_t>(srcSize.y),
                static_cast<uint32_t>(outputSize.x), static_cast<uint32_t>(outputSize.y),
                isHDR);
        }
    }

    class ThumbnailRenderer
    {
    public:
        virtual ~ThumbnailRenderer() = default;
        virtual void Render(Ref<ThumbnailPoolTask> task) = 0;

        static void Init();
        static void Shutdown();
        static void RenderTask(Ref<ThumbnailPoolTask> task);

        template <typename T>
		static T* GetRenderer();

        static ThumbnailRenderer* GetRenderer(AssetType type) {
            CZ_CORE_ASSERT(s_Renderers[type], std::string(Utils::AssetTypeToString(type)) + "ThumbnailRenderer has not been initialized.");
            return s_Renderers[type].get();
        }
        static uint32_t GetThumbnailSize() { return s_ThumbnailSize; }
    private:
        static std::unordered_map<AssetType, Scope<ThumbnailRenderer>> s_Renderers;
        static uint32_t s_ThumbnailSize;
    };

    //==============================================================================
    // SceneThumbnailRenderer
    class SceneThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        SceneThumbnailRenderer();
        void Render(Ref<ThumbnailPoolTask> task) override;
    private:
        glm::vec2 m_ViewportSize{200, 200};
        EditorCamera m_Camera;
        Ref<Scene> m_Scene;
		Ref<SceneRenderer> m_SceneRenderer;
    };

    //==============================================================================
    // TextureThumbnailRenderer
    class TextureThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        void Render(Ref<ThumbnailPoolTask> task) override;
    };

    //==============================================================================
    // MaterialThumbnailRenderer
    using MaterialProp = std::variant<UniformValue, Ref<Texture2D>>;

    class MaterialThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        MaterialThumbnailRenderer();
        void Render(Ref<ThumbnailPoolTask> task) override;

        void Update();
    
        Ref<Material> GetMaterial() { return m_Material; }
        void SetMaterial(const Ref<Material>& material);
        void SetMaterialProp(const std::string& name, const MaterialProp& value);
        glm::vec2 GetViewportSize() const { return m_ViewportSize; }
        Ref<Scene> GetScene() const { return m_Scene; }
        Ref<SceneRenderer> GetSceneRenderer() { return m_SceneRenderer; }
        Ref<Texture2D> GetOutput() { return m_Cache ? m_Cache : m_SceneRenderer->GetCompositePass()->GetOutput(0); }
        Ref<Texture2D> GetCache() const { return m_Cache; }
    private:
        void UpdateCache();
        Entity GetSphere();
    private:
        glm::vec2 m_ViewportSize{200, 200};
        EditorCamera m_Camera;
        Ref<Scene> m_Scene;
		Ref<SceneRenderer> m_SceneRenderer;
        Ref<Material> m_Material;
        Ref<Texture2D> m_Cache;
        bool m_Updated = false;
    };

    //==============================================================================
    // MeshSourceThumbnailRenderer
    class MeshSourceThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        MeshSourceThumbnailRenderer();
        void Render(Ref<ThumbnailPoolTask> task) override;
    private:
        glm::vec2 m_ViewportSize{200, 200};
        EditorCamera m_Camera;
        Ref<Scene> m_Scene;
		Ref<SceneRenderer> m_SceneRenderer;
    };
    
} // namespace Chozo

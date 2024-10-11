#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Scene/Scene.h"
#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/SceneRenderer.h"
#include "Chozo/Renderer/Texture.h"
#include "Chozo/Renderer/Material.h"

namespace Chozo {
    
    class ThumbnailRenderer : public RefCounted
    {
    public:
        virtual Ref<Texture2D> Render(AssetMetadata metadata, Ref<Asset> asset) = 0;
    };

    class TextureThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        virtual Ref<Texture2D> Render(AssetMetadata metadata, Ref<Asset> asset) override;
    };

    class MaterialThumbnailRenderer : public ThumbnailRenderer
    {
    public:
        MaterialThumbnailRenderer();
        virtual Ref<Texture2D> Render(AssetMetadata metadata, Ref<Asset> asset) override;

        void OnUpdate();
    
        Entity GetSphere();
        Ref<Material> GetMaterial();
        void SetMaterial(Ref<Material> material);
        void SetMaterialValue(Ref<Material> material, std::string name, UniformValue value);
        inline Ref<Scene> GetScene() { return m_Scene; }
        inline Ref<Texture2D> GetOutput() { return m_SceneRenderer->GetCompositePass()->GetOutput(0); }
    private:
        Ref<Scene> m_Scene;
		Ref<SceneRenderer> m_SceneRenderer;
        EditorCamera m_Camera;
    };
    
} // namespace Chozo

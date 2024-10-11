#pragma once

#include "Chozo.h"
#include "Chozo/Thumbnail/ThumbnailExporter.h"

namespace Chozo {

    class MaterialPanel
    {
    public:
        MaterialPanel();
        MaterialPanel(const Ref<Scene> context);

        inline static void SetMaterial(Ref<Material> material)
        {
            ThumbnailExporter::GetMaterialThumbnailRenderer()->SetMaterial(material);
            ThumbnailExporter::GetMaterialThumbnailRenderer()->OnUpdate();
            s_Instance->m_Material = material;
            s_Instance->m_PreviewUpdated = true;
        }
        inline static Ref<Scene> GetContext() { return s_Instance->m_Context; }
        inline static void SetContext(const Ref<Scene> context) { s_Instance->m_Context = context; }

        inline static void Open() { s_Show = true; }
        inline static void Close() { s_Show = false; }
        void OnImGuiRender();
    private:
        void RenderPreviewImage(Ref<Texture2D> texture = nullptr);

        void SetThumbnailMaterial(Entity entity);
        void OnMaterialChange(Ref<Material> material, std::string name, UniformValue value);
    private:
		static MaterialPanel* s_Instance;
        static bool s_Show;

        Ref<Scene> m_Context;
        Ref<Material> m_Material;
        Ref<Texture2D> m_PreviewCache;
        bool m_PreviewUpdated;
    };
}

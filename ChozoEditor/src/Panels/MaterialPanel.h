#pragma once

#include "Chozo.h"

namespace Chozo {

    #define FOREACH_PREVIEW_TYPE(TYPE) \
        	TYPE(Albedo) \
        	TYPE(Metalness) \
        	TYPE(Roughness) \
        	TYPE(Normal) \

	enum class PreviewType : uint16_t {
		#define GENERATE_ENUM(ENUM) ENUM,
		FOREACH_PREVIEW_TYPE(GENERATE_ENUM)
		#undef GENERATE_ENUM
		None
	};

    inline PreviewType StringToPreviewType(std::string_view type) {
        #define GENERATE_IF(ENUM) if (type == #ENUM) return PreviewType::ENUM;
        FOREACH_PREVIEW_TYPE(GENERATE_IF)
        #undef GENERATE_IF
        return PreviewType::None;
    }

    inline const char* PreviewTypeToString(PreviewType type) {
        switch (type) {
            #define GENERATE_CASE(ENUM) case PreviewType::ENUM: return #ENUM;
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return "Unknown";
        }
    }

    class MaterialPanel
    {
    public:
        MaterialPanel();
        MaterialPanel(const Ref<Scene> context);

        static void Init();

        static void SetMaterial(Ref<Material> material);
        inline static Ref<Scene> GetContext() { return s_Instance->m_Context; }
        inline static void SetContext(const Ref<Scene> context) { s_Instance->m_Context = context; }

        inline static void Open() { s_Show = true; }
        inline static void Close() { s_Show = false; }
        void OnImGuiRender();
    private:
        Ref<Texture2D>& GetPreviewTextureByType(PreviewType type);
        void UpdatePreviewTextureByType(PreviewType type);

        void RenderTextureProp(PreviewType type);
        void RenderPreviewImageByType(PreviewType type);
        void RenderPreviewImage(PreviewType type = PreviewType::None, Ref<Texture2D> texture = nullptr);

        void OnMaterialChange(Ref<Material> material, std::string name, Ref<Texture2D> texture);
        void OnMaterialChange(Ref<Material> material, std::string name, UniformValue value);
    private:
		static MaterialPanel* s_Instance;
        static bool s_Show;

        Ref<Scene> m_Context;
        Ref<Material> m_Material;
        Ref<Texture2D> m_AlbedoTexture, m_MetalnessTexture, m_RoughnessTexture, m_NormalTexture;
        bool m_PreviewUpdated;
    };
}

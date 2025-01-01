#pragma once

#include "Chozo.h"
#include "Thumbnail/ThumbnailRenderer.h"

namespace Chozo {

    #define FOREACH_PREVIEW_TYPE(TYPE) \
        	TYPE(BaseColor) \
        	TYPE(MetallicRoughness) \
            TYPE(Occlusion) \
            TYPE(Emissive) \
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

        static void Init();
        static void SetMaterial(AssetHandle handle);
        inline static void Open() { s_Show = true; }
        inline static void Close() { s_Show = false; }

        void OnImGuiRender();
    private:
        void RenderTextureProp(PreviewType type);
        void RenderPreviewImageByType(PreviewType type) const;

        void RenderPreviewImage(PreviewType type = PreviewType::None, const Ref<Texture2D>& texture = nullptr);

        Ref<Texture2D>& GetPreviewTextureByType(PreviewType type);
        void UpdatePreviewTextureByType(PreviewType type);

        static void OnMaterialChange(const std::string& name, const MaterialProp& value);
    private:
		static MaterialPanel* s_Instance;
        static bool s_Show;

        AssetHandle m_Material;
        Ref<Texture2D> m_BaseColorTexture, m_MetallicRoughnessTexture, m_OcclusionTexture, m_EmissiveTexture, m_NormalTexture;
    };
}

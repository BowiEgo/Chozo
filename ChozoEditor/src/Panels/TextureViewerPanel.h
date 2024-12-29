#pragma once

#include "Chozo.h"

namespace Chozo {

    class TextureViewerPanel
    {
    public:
        TextureViewerPanel();

        static void SetTexture(const Ref<Texture2D>& texture);
        static void SetTexture(const Ref<TextureCube>& texture);
        static inline void Open() { s_Show = true; }
        static inline void Close() { s_Show = false; }

        void OnImGuiRender();
    private:
        void OnDragAndDrop();
        void DrawImage(Ref<Texture2D> texture) const;
    private:
        Ref<Pipeline> m_Pipeline;
        Ref<Material> m_Material;

        Ref<Framebuffer> m_Viewport_FBO;
        ImVec2 m_ViewportSize;

        Ref<Texture> m_Texture;
        Ref<Texture2D> m_FBOTexture;
    private:
        static bool s_Show;
        static TextureViewerPanel* s_Instance;
    };
}
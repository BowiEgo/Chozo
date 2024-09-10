#pragma once

#include "Chozo.h"

namespace Chozo {

    class TexturePreviewPanel
    {
    public:
        TexturePreviewPanel();

        static void SetTexture(Ref<Texture2D>& texture) { s_Instance->m_Texture = texture; }
        static void SetTexture(Ref<TextureCube>& texture) { s_Instance->m_Texture = texture; }
        static inline void Open() { s_Show = true; }
        static inline void Close() { s_Show = false; }

        void OnImGuiRender();
    private:
        void DrawImage(Ref<Texture2D> texture);
    private:
        Ref<Framebuffer> m_Viewport_FBO;
        ImVec2 m_ViewportSize;

        Ref<Texture> m_Texture;
        Ref<Texture2D> m_FBOTexture;
    private:
        static bool s_Show;
        static TexturePreviewPanel* s_Instance;
    };
}
#include "TexturePreviewPanel.h"

#include "Chozo/ImGui/ImGuiUI.h"
#include <regex>
// #include <imgui_internal.h>

namespace Chozo {

    TexturePreviewPanel* TexturePreviewPanel::s_Instance = nullptr;
    bool TexturePreviewPanel::s_Show = false;

    extern const std::filesystem::path g_AssetsPath;
    extern const std::regex imagePattern;

    TexturePreviewPanel::TexturePreviewPanel()
    {
        s_Instance = this;

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { ImageFormat::RGBA8 };
        m_Viewport_FBO = Framebuffer::Create(fbSpec);

        TextureSpecification texSpec;
        texSpec.Width = m_ViewportSize.x;
        texSpec.Height = m_ViewportSize.y;
        m_FBOTexture = Texture2D::Create(m_Viewport_FBO->GetColorAttachmentRendererID(0), texSpec);
    }

    void TexturePreviewPanel::OnImGuiRender()
    {
        if (s_Show)
        {
            // Resize
            if (FramebufferSpecification spec = m_Viewport_FBO->GetSpecification();
                m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero size framebuffer is invalid
                (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
            {
                m_Viewport_FBO->Resize(m_ViewportSize.x, m_ViewportSize.y);
            }

            ImGui::SetNextWindowSize(ImVec2(800, 600));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
            ImGui::Begin("Texture Preview", &s_Show);

            m_ViewportSize = ImGui::GetContentRegionAvail();

            ImGui::InvisibleButton("##DragDropTarget", m_ViewportSize);
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    CZ_INFO("Drop target: {0}", (char*)path);
                    std::filesystem::path filePath = std::filesystem::path((char*)path);
                    std::string fileExtension = filePath.extension().string();
                    if (std::regex_match(fileExtension, imagePattern))
                    {
                        std::filesystem::path texturePath = g_AssetsPath / std::filesystem::path((char*)path);

                        TextureSpecification spec;
                        spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
                        spec.WrapT = ImageParameter::CLAMP_TO_BORDER;
                        m_Texture = Texture2D::Create(texturePath.string(), spec);
                    }
                }

                ImGui::EndDragDropTarget();
            }

            if (m_Texture && dynamic_cast<Texture2D*>(m_Texture.get()))
                DrawImage(std::dynamic_pointer_cast<Texture2D>(m_Texture));

            if (m_Texture && dynamic_cast<TextureCube*>(m_Texture.get()))
            {
                // CZ_CORE_INFO("TextureCube: {0}", m_Texture->GetRendererID());
                // float width = m_ViewportSize.x;
                // float height = m_ViewportSize.x * 0.75;
                // m_Viewport_FBO->Resize(width, height);
                m_Viewport_FBO->Bind();
                Renderer2D::DrawFullScreenQuad(m_Texture);
                m_Viewport_FBO->Unbind();

                DrawImage(m_FBOTexture);
                // CZ_CORE_INFO("{0}, {1}", m_FBOTexture->GetWidth(), m_FBOTexture->GetHeight());
            }

            ImGui::End();
            ImGui::PopStyleVar();
        }
    }

    void TexturePreviewPanel::DrawImage(Ref<Texture2D> texture)
    {
        float viewportAspectRatio = m_ViewportSize.y / m_ViewportSize.x;
        float imageAspectRatio = static_cast<float>(texture->GetHeight()) / static_cast<float>(texture->GetWidth());
        ImVec2 uv0(0.0f, 1.0f);
        ImVec2 uv1(1.0f, 0.0f);

        if (imageAspectRatio <= viewportAspectRatio) {
            float scaleY = viewportAspectRatio / imageAspectRatio;
            float offsetY = (1.0f - scaleY) / 2.0f;
            uv0.y = 1.0f - offsetY;
            uv1.y = offsetY;
        } else if (imageAspectRatio > viewportAspectRatio) {
            float scaleX = imageAspectRatio / viewportAspectRatio;
            float offsetX = (1.0f - scaleX) / 2.0f;
            uv0.x = offsetX;
            uv1.x = 1.0f - offsetX;
        }

        UI::DrawButtonImage(texture, IM_COL32(255, 255, 255, 225), UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f), uv0, uv1);
    }
}
#include "TextureViewerPanel.h"

#include <regex>
// #include <imgui_internal.h>
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/Utilities/StringUtils.h"

namespace Chozo {

    TextureViewerPanel* TextureViewerPanel::s_Instance = nullptr;
    bool TextureViewerPanel::s_Show = false;

    extern const std::filesystem::path g_AssetsPath;
    extern const std::regex imagePattern;

    TextureViewerPanel::TextureViewerPanel()
    {
        s_Instance = this;

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { ImageFormat::RGBA8 };
        m_Viewport_FBO = Framebuffer::Create(fbSpec);

        Texture2DSpecification texSpec;
        texSpec.Width = m_ViewportSize.x;
        texSpec.Height = m_ViewportSize.y;
        m_FBOTexture = Texture2D::Create(m_Viewport_FBO->GetColorAttachmentRendererID(0), texSpec);

        PipelineSpecification pipelineSpec;
        pipelineSpec.DebugName = "TextureViewer";
        pipelineSpec.Shader = Renderer::GetRendererData().m_ShaderLibrary->Get("CubemapPreview");
        pipelineSpec.DepthWrite = false;
        pipelineSpec.DepthTest = false;
        pipelineSpec.Layout = {
            { ShaderDataType::Float3, "a_Position" },
        };
        pipelineSpec.TargetFramebuffer = m_Viewport_FBO;

        m_Material = Material::Create(pipelineSpec.Shader, pipelineSpec.DebugName);
        m_Pipeline = Pipeline::Create(pipelineSpec);
    }

    void TextureViewerPanel::SetTexture(Ref<Texture2D> &texture)
    {
        s_Instance->m_Texture = texture;
    }

    void TextureViewerPanel::SetTexture(Ref<TextureCube> &texture)
    {
        s_Instance->m_Texture = texture;
    }

    void TextureViewerPanel::OnImGuiRender()
    {
        if (s_Show)
        {
            // Resize
            m_Viewport_FBO->Resize(m_ViewportSize.x, m_ViewportSize.y);

            ImGui::SetNextWindowSize(ImVec2(800, 600));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
            ImGui::Begin("Texture Viewer", &s_Show);

            m_ViewportSize = ImGui::GetContentRegionAvail();
            m_ViewportSize.x += 2;
            m_ViewportSize.y += 2;

            OnDragAndDrop();

            if (m_Texture && dynamic_cast<Texture2D*>(m_Texture.get()))
                DrawImage(m_Texture.As<Texture2D>());

            if (m_Texture && dynamic_cast<TextureCube*>(m_Texture.get()))
            {
                // CZ_CORE_INFO("TextureCube: {0}", m_Texture->GetRendererID());
                // float width = m_ViewportSize.x;
                // float height = m_ViewportSize.x * 0.75;
                // m_Viewport_FBO->Resize(width, height);

                // TODO: Change to Async Commandbuffer
                m_Viewport_FBO->Bind();
                m_Material->Set("u_Texture", m_Texture);
                Renderer::GetAPI()->RenderFullscreenQuad(m_Pipeline, m_Material);
                m_Viewport_FBO->Unbind();

                DrawImage(m_FBOTexture);
                // CZ_CORE_INFO("{0}, {1}", m_FBOTexture->GetWidth(), m_FBOTexture->GetHeight());
            }

            ImGui::PopStyleVar();
            ImGui::End();
        }
    }

    void TextureViewerPanel::OnDragAndDrop()
    {
        ImGui::InvisibleButton("##DragDropTarget", m_ViewportSize);
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* handle_wchar = (const wchar_t*)payload->Data;
                AssetHandle handle = Utils::WChar::WCharToUint64(handle_wchar);
                CZ_INFO("Drop target: {0}", std::to_string(handle));

                Ref<Asset> asset = Application::GetAssetManager()->GetAsset(handle);
                if (asset->GetAssetType() == AssetType::Texture)
                    m_Texture = asset.As<Texture2D>();
            }

            ImGui::EndDragDropTarget();
        }
    }

    void TextureViewerPanel::DrawImage(Ref<Texture2D> texture)
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
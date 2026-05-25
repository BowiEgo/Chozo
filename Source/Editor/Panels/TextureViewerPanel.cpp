#include "TextureViewerPanel.h"

void TextureViewerPanel::Draw(const char* title) {
    if (!m_bOpen) return;

    if (!ImGui::Begin(title, &m_bOpen)) {
        ImGui::End();
        return;
    }

    ImGui::End();
}
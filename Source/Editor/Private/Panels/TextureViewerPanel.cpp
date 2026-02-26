#include "TextureViewerPanel.h"

void TextureViewerPanel::Draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    ImGui::End();
}
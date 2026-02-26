#include "MaterialPanel.h"

void MaterialPanel::Draw(const char* title, bool* p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    ImGui::End();
}
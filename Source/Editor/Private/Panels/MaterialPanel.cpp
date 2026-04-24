#include "MaterialPanel.h"

#include "Controls.h"

void MaterialPanel::Draw(const char* title) {
    if (!m_bOpen) return;

    if (!ImGui::Begin(title, &m_bOpen)) {
        ImGui::End();
        return;
    }

    if (m_Material) {
        IParams* params = m_Material->GetParams().Get();
        if (DrawColumnProperties("Props", params)) {
        }
    }

    ImGui::End();
}

bool MaterialPanel::DrawColumnProperties(const std::string& name, IParams* params) {
    bool valChanged = false;
    if (constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable;
        ImGui::BeginTable("table", 2, flags)) {

        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        ChozoEditor::Controls::TableParamsVisitor visitor;
        params->Accept(visitor);

        if (visitor.IsValueChanged()) {
            valChanged = true;
        }

        ImGui::EndTable();
    }

    return valChanged;
}
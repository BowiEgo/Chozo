#include "PropertiesPanel.h"

#include "Controls.h"

void PropertiesPanel::Draw(const char* title) {
    if (!m_bOpen) return;

    if (!ImGui::Begin(title, &m_bOpen)) {
        ImGui::End();
        return;
    }

    if (m_NodeTree) {
        auto node = m_NodeTree->GetSelectedNode();
        if (node) {
            DrawInfoProperties(node);
            DrawTransformProperties(node);
            DrawHDRIBackdropProperties(node);
            DrawMeshProperties(node);
        }
    }

    ImGui::End();
}

void PropertiesPanel::DrawComponentHeader(const std::string& name, bool bDefaultOpen,
                                          const DrawContentFunc& drawContentFunc) {
    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowOverlap |
                                       ImGuiTreeNodeFlags_SpanAvailWidth |
                                       ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

    if (bDefaultOpen) treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    const float lineHeight        = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;

    ImGui::Separator();
    bool open =
        ImGui::TreeNodeEx((void*)std::hash<std::string>{}(name), treeNodeFlags, "%s", name.c_str());

    ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
    if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight })) {
    }

    if (open) {
        ImGui::Indent();
        drawContentFunc();
        ImGui::Unindent();
        ImGui::TreePop();
    }
}

bool PropertiesPanel::DrawColumnProperties(const std::string& name, IParams* params) {
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

void PropertiesPanel::DrawInfoProperties(FEditorNode* node) {
    if (!node) return;

    DrawComponentHeader("Info", true,
                        [this, node]() { ImGui::Text("%s", node->GetName().c_str()); });
}

void PropertiesPanel::DrawTransformProperties(FEditorNode* node) {
    if (!node) return;
    if (!node->HasTransform()) return;

    DrawComponentHeader("Transform", true, [this, node]() {
        auto params = node->GetTransformParams();
        if (DrawColumnProperties("Transform", params)) {
            node->MarkDirty();
        }
    });
}

void PropertiesPanel::DrawHDRIBackdropProperties(FEditorNode* node) {
    if (!node) return;
    if (!node->HasHDRIBackdrop()) return;

    DrawComponentHeader("HDRI Backdrop", true, [this, node]() {
        auto params = node->GetHDRIBackdropParams();
        if (DrawColumnProperties("HDRI Backdrop", params)) {
            node->MarkDirty();
        }
    });
}

void PropertiesPanel::DrawMeshProperties(FEditorNode* node) {
    if (!node) return;
    if (!node->HasMesh()) return;

    DrawComponentHeader("Mesh", true, [this, node]() {
        auto params = node->GetMeshParams()->Get();
        if (DrawColumnProperties("Mesh", params)) {
            node->MarkDirty();
        }
    });
}

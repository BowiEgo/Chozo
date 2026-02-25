#include "SceneHierarchyPanel.h"

#include <imgui_internal.h>

DEFINE_LOG_CATEGORY(LogSceneHierarchyPanel);

void SceneHierarchyPanel::Draw(const char* title, bool* p_open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);

    if (ImGui::InputTextWithHint("##Filter", "incl,-excl", m_Filter.InputBuf,
                                 IM_ARRAYSIZE(m_Filter.InputBuf),
                                 ImGuiInputTextFlags_EscapeClearsAll))
        m_Filter.Build();
    ImGui::PopItemFlag();

    if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg)) {
        for (TreeNode* node : m_RootNode->Childs) {
            if (m_Filter.PassFilter(node->Name)) {
                DrawTreeNode(node);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void SceneHierarchyPanel::DrawTreeNode(TreeNode* node) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGuiID node_id = ImGui::GetID((void*)(intptr_t)node->UID);
    ImGui::PushID(node->UID);

    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
    tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow |
                  ImGuiTreeNodeFlags_OpenOnDoubleClick; // Standard opening mode as we are likely to
                                                        // want to add selection afterwards
    tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent; // Left arrow support
    tree_flags |= ImGuiTreeNodeFlags_SpanFullWidth;        // Span full width for easier mouse reach
    tree_flags |= ImGuiTreeNodeFlags_DrawLinesToNodes;     // Always draw hierarchy outlines
    if (node == m_SelectedNode) tree_flags |= ImGuiTreeNodeFlags_Selected;
    if (node->Childs.Size == 0) tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    if (node->DataMyBool == false)
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

    bool node_open = ImGui::TreeNodeBehavior(node_id, tree_flags, node->Name);

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
        m_SelectedNode = node;
        ImGui::SetKeyboardFocusHere(-1);
    }

    if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter) &&
        !ImGui::IsItemActivated()) {
        ImGui::ActivateItemByID(node_id);
    }

    if (ImGui::IsItemActivated() || ImGui::IsItemFocused()) {
        m_SelectedNode = node;
    }

    if (node->DataMyBool == false) ImGui::PopStyleColor();
    // --- Context Menu Begin ---

    if (ImGui::BeginPopupContextItem()) {
        m_SelectedNode = node; // Right-click also selects the node
        if (ImGui::MenuItem("Rename")) {
            // Trigger rename logic (e.g., set an 'IsEditing' flag)
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete", "Delete Key", false, node != m_RootNode)) {
            // Logic to remove node from parent's Childs vector
            // Note: Actual deletion should happen outside the rendering loop to avoid crash
            // PendingDeleteNode = node;
        }
        ImGui::EndPopup();
    }
    // --- Context Menu End ---

    if (node_open) {
        for (TreeNode* child : node->Childs)
            DrawTreeNode(child);
        ImGui::TreePop();
    }
    ImGui::PopID();
}
#include "SceneHierarchyPanel.h"

#include <imgui_internal.h>

DEFINE_LOG_CATEGORY(LogSceneHierarchyPanel);

void SceneHierarchyPanel::Draw(const char* title, bool* p_open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    // Filter input
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);

    if (ImGui::InputTextWithHint("##Filter", "incl,-excl", m_Filter.InputBuf,
                                 IM_ARRAYSIZE(m_Filter.InputBuf),
                                 ImGuiInputTextFlags_EscapeClearsAll)) {
        m_Filter.Build();
    }
    ImGui::PopItemFlag();

    // Flattern Tree
    if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg)) {
        m_FlattenedView.clear();
        FlattenTree(m_RootNode, 0);

        ImGuiListClipper clipper;
        clipper.Begin((int)m_FlattenedView.size());

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                FlattenedNode& item = m_FlattenedView[i];
                DrawFlattenedNode(item.Node, item.Depth);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void SceneHierarchyPanel::FlattenTree(TreeNode* node, int depth) {
    if (!node) return;

    if (node != m_RootNode) {
        m_FlattenedView.push_back({ node, depth });
    }

    ImGuiID node_id = ImGui::GetID((void*)(intptr_t)node->UID);
    bool is_open = ImGui::GetStateStorage()->GetInt(node_id, 0) != 0;

    if (is_open || node == m_RootNode) {
        for (TreeNode* child : node->Childs) {
            if (m_Filter.PassFilter(child->Name)) {
                FlattenTree(child, depth + (node == m_RootNode ? 0 : 1));
            }
        }
    }
}

void SceneHierarchyPanel::DrawFlattenedNode(TreeNode* node, int depth) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    // --- Node Lines Begin ---
    ImVec2 screen_pos = ImGui::GetCursorScreenPos();
    float indent_step = ImGui::GetStyle().IndentSpacing;
    float row_height = ImGui::GetTextLineHeightWithSpacing();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 line_color = ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.8f);

    for (int i = 0; i < depth; i++) {
        // Draw vertical lines for each level of depth.
        float line_x = screen_pos.x + (i + 0.5f) * indent_step + 10.0f;
        float start_y = screen_pos.y - row_height * 0.5;
        float end_y = screen_pos.y + row_height * 0.5;
        draw_list->AddLine(ImVec2(line_x, start_y), ImVec2(line_x, end_y), line_color);
    }
    // Draw the horizontal "stub" for the current node's depth
    float start_x = screen_pos.x + 0.5 * indent_step + 10.0f;
    float end_x = start_x + depth * indent_step;
    float stub_y = screen_pos.y + row_height * 0.5f;
    draw_list->AddLine(ImVec2(start_x, stub_y), ImVec2(end_x, stub_y), line_color);
    // --- Node Lines End ---

    // --- TreeNode Begin ---
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (depth * indent_step));

    ImGuiID node_id = ImGui::GetID((void*)(intptr_t)node->UID);
    ImGui::PushID(node->UID);

    ImGuiTreeNodeFlags tree_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // IMPORTANT: Use ImGuiTreeNodeFlags_NoTreePushOnOpen
                                             // because we are handling the "recursion" manually via
                                             // flattening.

    if (node == m_SelectedNode) tree_flags |= ImGuiTreeNodeFlags_Selected;
    if (node->Childs.Size == 0) tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    bool is_open = ImGui::TreeNodeBehavior(node_id, tree_flags, node->Name);

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
    // --- TreeNode End ---

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

    ImGui::PopID();
}

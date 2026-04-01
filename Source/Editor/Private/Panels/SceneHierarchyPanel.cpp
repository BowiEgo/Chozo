#include "SceneHierarchyPanel.h"

#include <imgui_internal.h>

DEFINE_LOG_CATEGORY(LogSceneHierarchyPanel);

SceneHierarchyPanel::SceneHierarchyPanel() {}

SceneHierarchyPanel::~SceneHierarchyPanel() {}

void SceneHierarchyPanel::Draw(const char* title, bool* p_open) {
    if (!m_NodeTree) return;

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

    if (ImGui::Button("+")) {
        ImGui::OpenPopup("CreateNodePopup");
    }

    ImGui::SameLine();
    if (ImGui::Button("-")) {
    }

    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Right-click on items for more options");
        ImGui::EndTooltip();
    }

    // ===== Popup =====
    if (ImGui::BeginPopup("CreateNodePopup")) {
        auto nodeBit = FRegistryManager::Get().GetBit("Node_Regular");
        auto selectedNode = m_NodeTree->GetSelectedNode();

        if (ImGui::MenuItem("Empty Node")) {
            auto emptyBit = FRegistryManager::Get().GetBit("Node_Empty");
            CreateNode("Empty", nodeBit |= emptyBit, selectedNode);
        }
        if (ImGui::MenuItem("Sphere")) {
            auto sphereBit = FRegistryManager::Get().GetBit("Mesh_Sphere");
            CreateNode("Sphere", nodeBit |= sphereBit, selectedNode);
        }
        if (ImGui::MenuItem("Cube")) {
            auto cubeBit = FRegistryManager::Get().GetBit("Mesh_Cube");
            CreateNode("Cube", nodeBit |= cubeBit, selectedNode);
        }
        if (ImGui::MenuItem("Cylinder")) {
        }
        ImGui::EndPopup();
    }

    // Flattern Tree
    if (ImGui::BeginTable("##HierarchyTable", 1, ImGuiTableFlags_RowBg)) {
        m_FlattenedView.clear();
        FlattenTree(m_NodeTree->GetRoot(), 0);

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

void SceneHierarchyPanel::FlattenTree(FEditorNode* node, int depth) {
    if (!node) return;

    bool isRoot = node->IsRoot();

    if (m_Filter.IsActive()) {
        if (!isRoot && m_Filter.PassFilter(node->GetName().c_str()))
            m_FlattenedView.push_back({ node, depth });

        for (FEditorNode* child : node->GetChildren()) {
            FlattenTree(child, depth + (isRoot ? 0 : 1));
        }
    } else {
        if (!isRoot) m_FlattenedView.push_back({ node, depth });

        if (node->IsOpen() || isRoot) {
            for (FEditorNode* child : node->GetChildren()) {
                if (m_Filter.PassFilter(child->GetName().c_str())) {
                    FlattenTree(child, depth + (isRoot ? 0 : 1));
                }
            }
        }
    }
}

void SceneHierarchyPanel::FlattenTreeFiltered(FEditorNode* node, int depth) {
    if (!node) return;

    bool isRoot = node->IsRoot();

    if (!isRoot && m_Filter.PassFilter(node->GetName().c_str())) {
        m_FlattenedView.push_back({ node, depth });
    }

    for (FEditorNode* child : node->GetChildren()) {
        FlattenTreeFiltered(child, depth + (isRoot ? 0 : 1));
    }
}

void SceneHierarchyPanel::DrawFlattenedNode(FEditorNode* node, int depth) {
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

    ImGuiID node_id = ImGui::GetID((void*)(intptr_t)node->GetID());
    ImGui::PushID(node->GetID());

    ImGuiTreeNodeFlags tree_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // IMPORTANT: Use ImGuiTreeNodeFlags_NoTreePushOnOpen
                                             // because we are handling the "recursion" manually via
                                             // flattening.

    if (node == m_NodeTree->GetSelectedNode()) tree_flags |= ImGuiTreeNodeFlags_Selected;
    if (node->GetChildren().empty())
        tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    ImGui::SetNextItemOpen(node->IsOpen(), ImGuiCond_Always);
    ImGui::TreeNodeBehavior(node_id, tree_flags, node->GetName().c_str());

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsItemToggledOpen()) {
        m_NodeTree->SelectNode(node);
        ImGui::SetKeyboardFocusHere(-1);
    }

    if (ImGui::IsItemToggledOpen() || ImGui::IsItemFocused() &&
                                          ImGui::IsKeyPressed(ImGuiKey_Enter) &&
                                          !ImGui::IsItemActivated()) {
        node->ToggleOpen();
    }

    // --- TreeNode End ---

    // --- Context Menu Begin ---
    if (ImGui::BeginPopupContextItem()) {
        m_NodeTree->SelectNode(node); // Right-click also selects the node
        if (ImGui::MenuItem("Rename")) {
            // Trigger rename logic (e.g., set an 'IsEditing' flag)
        }
        if (ImGui::MenuItem("Delete", "Delete Key", false, node != m_NodeTree->GetRoot())) {
            // Logic to remove node from parent's Childs vector
            // Note: Actual deletion should happen outside the rendering loop to avoid crash
            // PendingDeleteNode = node;
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Camera")) {
            }
            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Directional")) {
                }
                if (ImGui::MenuItem("Point")) {
                }
                if (ImGui::MenuItem("Spot")) {
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Geometry")) {
                auto nodeBit = FRegistryManager::Get().GetBit("Node_Regular");

                if (ImGui::MenuItem("Plane")) {
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Sphere")) {
                    auto sphereBit = FRegistryManager::Get().GetBit("Mesh_Sphere");
                    CreateNode("Sphere", nodeBit |= sphereBit, node);

                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Cube")) {
                    auto cubeBit = FRegistryManager::Get().GetBit("Mesh_Cube");
                    CreateNode("Cube", nodeBit |= cubeBit, node);

                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Cylinder")) {
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Cone")) {
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Cylinder")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
    // --- Context Menu End ---
}

void SceneHierarchyPanel::CreateNode(const std::string name, const uint32_t typeBit,
                                     FEditorNode* parent) {
    auto newNode = m_NodeTree->CreateNode(name, typeBit);

    if (parent) {
        newNode->SetParent(parent);
        parent->Open(); // Ensure parent is open to show the new child
    }
    m_NodeTree->SelectNode(newNode);
}
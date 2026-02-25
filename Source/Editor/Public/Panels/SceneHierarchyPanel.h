#pragma once

#include "Panel.h"

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSceneHierarchyPanel, Info);

struct TreeNode {
    // Tree structure
    char Name[28] = "";
    int UID = 0;
    TreeNode* Parent = NULL;
    ImVector<TreeNode*> Childs;
    unsigned short IndexInParent =
        0; // Maintaining this allows us to implement linear traversal more easily

    // Leaf Data
    bool HasData = false; // All leaves have data
    bool DataMyBool = true;
    int DataMyInt = 128;
    ImVec2 DataMyVec2 = ImVec2(0.0f, 3.141592f);
};

static TreeNode* CreateNode(const char* name, int uid, TreeNode* parent) {
    TreeNode* node = IM_NEW(TreeNode);
    snprintf(node->Name, IM_ARRAYSIZE(node->Name), "%s", name);
    node->UID = uid;
    node->Parent = parent;
    node->IndexInParent = parent ? (unsigned short)parent->Childs.Size : 0;
    if (parent) parent->Childs.push_back(node);
    return node;
}

static void DestroyNode(TreeNode* node) {
    for (TreeNode* child_node : node->Childs)
        DestroyNode(child_node);
    IM_DELETE(node);
}

static TreeNode* CreateDemoTree() {
    static const char* root_names[] = {
        "Apple",  "Banana", "Cherry",    "Kiwi",       "Mango",
        "Orange", "Pear",   "Pineapple", "Strawberry", "Watermelon"
    };
    const size_t NAME_MAX_LEN = sizeof(TreeNode::Name);
    char name_buf[NAME_MAX_LEN];
    int uid = 0;
    TreeNode* node_L0 = CreateNode("<ROOT>", ++uid, NULL);
    const int root_items_multiplier = 2;
    for (int idx_L0 = 0; idx_L0 < IM_ARRAYSIZE(root_names) * root_items_multiplier; idx_L0++) {
        snprintf(name_buf, IM_ARRAYSIZE(name_buf), "%s %d",
                 root_names[idx_L0 / root_items_multiplier], idx_L0 % root_items_multiplier);
        TreeNode* node_L1 = CreateNode(name_buf, ++uid, node_L0);
        const int number_of_childs = (int)strlen(node_L1->Name);
        for (int idx_L1 = 0; idx_L1 < number_of_childs; idx_L1++) {
            snprintf(name_buf, IM_ARRAYSIZE(name_buf), "Child %d", idx_L1);
            TreeNode* node_L2 = CreateNode(name_buf, ++uid, node_L1);
            node_L2->HasData = true;
            if (idx_L1 == 0) {
                snprintf(name_buf, IM_ARRAYSIZE(name_buf), "Sub-child %d", 0);
                TreeNode* node_L3 = CreateNode(name_buf, ++uid, node_L2);
                node_L3->HasData = true;
            }
        }
    }
    return node_L0;
}

class SceneHierarchyPanel : public Panel {
public:
    SceneHierarchyPanel() {}
    ~SceneHierarchyPanel() {}

    virtual void Draw(const char* title, bool* p_open) override;

    void DrawTreeNode(TreeNode* node);

private:
    ImGuiTextFilter m_Filter;
    TreeNode* m_RootNode = CreateDemoTree();
    TreeNode* m_SelectedNode = nullptr;
};
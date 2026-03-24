#pragma once

#include "CoreMinimal.h"

#include "EditorNode.h"
#include "EditorNodeTree.h"
#include "Panel.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSceneHierarchyPanel, Info);

struct FlattenedNode {
    FEditorNode* Node;
    int Depth;
};

class SceneHierarchyPanel : public Panel {
public:
    SceneHierarchyPanel();
    ~SceneHierarchyPanel();

    void SetNodeTree(CEditorNodeTree* tree) { m_NodeTree = tree; }

    virtual void Draw(const char* title, bool* p_open) override;

    void FlattenTree(FEditorNode* node, int depth);
    void DrawFlattenedNode(FEditorNode* node, int depth);

private:
    ImGuiTextFilter m_Filter;
    CEditorNodeTree* m_NodeTree;
    FEditorNode* m_PendingDeleteNode = nullptr;
    std::vector<FlattenedNode> m_FlattenedView;
};
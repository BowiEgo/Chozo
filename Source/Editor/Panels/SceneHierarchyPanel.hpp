#pragma once

#include "../EditorNode/EditorNode.hpp"
#include "../EditorNode/EditorNodeTree.hpp"
#include "Panel.hpp"

using namespace CZ;

struct FlattenedNode {
    EditorNode* Node;
    int Depth;
};

class SceneHierarchyPanel : public Panel {
public:
    SceneHierarchyPanel();
    ~SceneHierarchyPanel();

    void SetNodeTree(EditorNodeTree* tree) { m_NodeTree = tree; }
    virtual void Draw(const char* title) override;

private:
    void DrawNodeContextMenu(EditorNode* node);
    void DrawCreatingContextMenu(EditorNode* parent);
    void FlattenTree(EditorNode* node, int depth);
    void FlattenTreeFiltered(EditorNode* node, int depth);
    void DrawFlattenedNode(EditorNode* node, int depth);

    void CreateNode(const std::string name, const TypeMask typeMask, EditorNode* parent);

private:
    ImGuiTextFilter m_Filter;
    EditorNodeTree* m_NodeTree;
    EditorNode* m_PendingDeleteNode = nullptr;
    std::vector<FlattenedNode> m_FlattenedView;
};
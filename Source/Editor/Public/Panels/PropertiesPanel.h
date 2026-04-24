#pragma once

#include "EditorNodeTree.h"
#include "Panel.h"

class PropertiesPanel : public Panel {
    using DrawContentFunc = std::function<void()>;

public:
    PropertiesPanel() {}
    ~PropertiesPanel() {}

    virtual void Draw(const char* title) override;

    void SetNodeTree(CEditorNodeTree* nodeTree) { m_NodeTree = nodeTree; }

private:
    void DrawComponentHeader(const std::string& name, bool bDefaultOpen = false,
                             const DrawContentFunc& drawContentFunc = DrawContentFunc());
    bool DrawColumnProperties(const std::string& name, IParams* params);
    void DrawInfoProperties(FEditorNode* node);
    void DrawTransformProperties(FEditorNode* node);
    void DrawHDRIBackdropProperties(FEditorNode* node);
    void DrawMeshProperties(FEditorNode* node);

private:
    CEditorNodeTree* m_NodeTree;
};
#pragma once

#include "../EditorNode/EditorNodeTree.hpp"
#include "Panel.hpp"

using namespace CZ;

class PropertiesPanel : public Panel {
    using DrawContentFunc = std::function<void()>;

public:
    PropertiesPanel() {}
    ~PropertiesPanel() {}

    virtual void Draw(const char* title) override;

    void SetNodeTree(EditorNodeTree* nodeTree) { m_NodeTree = nodeTree; }

private:
    void DrawComponentHeader(const std::string& name, bool bDefaultOpen = false,
                             const DrawContentFunc& drawContentFunc = DrawContentFunc());
    bool DrawColumnProperties(const std::string& name, Params* params);
    void DrawInfoProperties(EditorNode* node);
    void DrawTransformProperties(EditorNode* node);
    void DrawHDRIBackdropProperties(EditorNode* node);
    void DrawMeshProperties(EditorNode* node);

private:
    EditorNodeTree* m_NodeTree;
};
#pragma once

#include "Panel.h"

class SceneHierarchyPanel : public Panel {
public:
    SceneHierarchyPanel() {}
    ~SceneHierarchyPanel() {}

    virtual void Draw(const char* title, bool* p_open) override;
};
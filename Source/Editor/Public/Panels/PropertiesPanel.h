#pragma once

#include "Panel.h"

class PropertiesPanel : public Panel {
public:
    PropertiesPanel() {}
    ~PropertiesPanel() {}

    virtual void Draw(const char* title, bool* p_open) override;
};
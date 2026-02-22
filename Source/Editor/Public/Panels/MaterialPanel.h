#pragma once

#include "Panel.h"

class MaterialPanel : public Panel {
public:
    MaterialPanel() {}
    ~MaterialPanel() {}

    virtual void Draw(const char* title, bool* p_open) override;
};
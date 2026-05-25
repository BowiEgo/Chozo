#pragma once

#include "Panel.h"

class AssetsPanel : public Panel {
public:
    AssetsPanel() {}
    ~AssetsPanel() {}

    virtual void Draw(const char* title) override;
};
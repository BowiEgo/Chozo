#pragma once

#include "Panel.hpp"

class AssetsPanel : public Panel {
public:
    AssetsPanel() {}
    ~AssetsPanel() {}

    virtual void Draw(const char* title) override;
};
#pragma once

#include "Panel.h"

class TextureViewerPanel : public Panel {
public:
    TextureViewerPanel() {}
    ~TextureViewerPanel() {}

    virtual void Draw(const char* title, bool* p_open) override;
};
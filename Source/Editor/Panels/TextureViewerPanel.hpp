#pragma once

#include "Panel.hpp"

class TextureViewerPanel : public Panel {
public:
    TextureViewerPanel() {}
    ~TextureViewerPanel() {}

    virtual void Draw(const char* title) override;
};
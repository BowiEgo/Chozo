#pragma once

#include "Panel.h"

class ContentBrowserPanel : public Panel {
public:
    ContentBrowserPanel() {}
    ~ContentBrowserPanel() {}

    virtual void Draw(const char* title, bool* p_open) override;
};
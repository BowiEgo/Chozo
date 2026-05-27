#pragma once

#include <Runtime/RHI/GraphicsContext.hpp>

using namespace CZ;

class Panel {
public:
    Panel() {};
    ~Panel() {};

    virtual void Draw(const char* title) = 0;

    void Open() { m_IsOpen = true; }

    void SetContext(GraphicsContext context) { m_Context = context; }

protected:
    GraphicsContext m_Context;

    bool m_IsOpen = false;
};

#pragma once

#include "RHIContext.h"

class Panel {
public:
    Panel() {};
    ~Panel() {};

    virtual void Draw(const char* title) = 0;

    void Open() { m_bOpen = true; }

    void SetContext(IRHIContext* context) { m_Context = context; }

protected:
    IRHIContext* m_Context;

    bool m_bOpen = false;
};
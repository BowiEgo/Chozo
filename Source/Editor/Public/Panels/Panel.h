#pragma once

#include "RHIContext.h"

class Panel {
public:
    Panel() {};
    ~Panel() {};

    virtual void Draw(const char* title, bool* p_open) = 0;

    void SetContext(IRHIContext* context) { m_Context = context; }

protected:
    IRHIContext* m_Context;
};
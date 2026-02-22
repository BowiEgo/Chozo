#pragma once

class Panel {
public:
    Panel() {};
    ~Panel() {};

    virtual void Draw(const char* title, bool* p_open) = 0;
};
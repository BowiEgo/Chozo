#pragma once

class RHI_API IRHI {
public:
    IRHI() {}

    virtual ~IRHI() = default;

    static TRef<IRHI> Create();
};
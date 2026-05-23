#pragma once

#include <Core/Header/Handle.hpp>

namespace CZ {

class FenceObj {
public:
    FenceObj() {};

    virtual ~FenceObj() {};

    virtual bool WaitAndReset(uint64_t timeout) const = 0;
};

struct Fence : Handle<class FenceObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

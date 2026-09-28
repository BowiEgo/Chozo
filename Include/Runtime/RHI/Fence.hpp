#pragma once
#include <cstdint>

#include <Core/Header/Handle.hpp>

namespace CZ {

class FenceObj {
public:
    FenceObj() {};

    virtual ~FenceObj() {};

    virtual bool WaitAndReset(uint64_t timeout) const = 0;
};

struct Fence : Handle<class FenceObj> {
    using Handle<class FenceObj>::Handle;

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

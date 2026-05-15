#pragma once

#include <Core/Header/Handle.hpp>

namespace CZ {

class FenceObj;

struct Fence : Handle<class FenceObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    bool WaitAndReset(uint64_t timeout) const;
};

} // namespace CZ

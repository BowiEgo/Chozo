#pragma once

#include <Core/Header/Handle.hpp>

namespace CZ {

class SemaphoreObj {
public:
    SemaphoreObj() = default;

    virtual ~SemaphoreObj() = default;
};

struct Semaphore : Handle<class SemaphoreObj> {
    using Handle<class SemaphoreObj>::Handle;

    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

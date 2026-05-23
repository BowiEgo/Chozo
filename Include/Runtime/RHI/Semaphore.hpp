#pragma once

#include <Core/Header/Handle.hpp>

namespace CZ {

class SemaphoreObj {
public:
    SemaphoreObj() = default;

    virtual ~SemaphoreObj() = default;
};

struct Semaphore : Handle<class SemaphoreObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }
};

} // namespace CZ

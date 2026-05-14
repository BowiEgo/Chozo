#pragma once

#include <Core/Header/Handle.h>

namespace CZ {

class SemaphoreObj;

struct Semaphore : Handle<class SemaphoreObj> {
    template <typename T> T* As() { return static_cast<T*>(RHIInternalReader::Unwrap(*this)); }
};

} // namespace CZ

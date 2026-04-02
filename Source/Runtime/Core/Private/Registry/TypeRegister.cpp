#include "TypeRegister.h"

FTypeRegister& FTypeRegister::Get() {
    static FTypeRegister Instance;
    return Instance;
}

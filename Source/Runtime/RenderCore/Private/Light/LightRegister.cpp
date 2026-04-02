#include "LightRegister.h"

FLightRegister& FLightRegister::Get() {
    static FLightRegister Instance;
    return Instance;
}

void FLightRegister::Init() { Get(); }
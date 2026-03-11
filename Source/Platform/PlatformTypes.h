#pragma once

#include <cstdint>
#include <string>

struct FRawFileImage {
    std::string Name;
    std::string PathU8;
    uint8_t* Data = nullptr;
    int Size = 0;
    uint32_t Width = 0, Height = 0;
    int Index = -1;
    char Format = 0; // { 0 = BGRA, 1 = RGBA }
};
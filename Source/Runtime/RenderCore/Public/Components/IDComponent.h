#pragma once

#include "CoreMinimal.h"
#include "UUID.h"

struct FIDComponent {
    FUUID ID;

    FIDComponent() = default;
    FIDComponent(const FIDComponent&) = default;
};
#pragma once

#include "CoreMinimal.h"
#include "UUID.h"

struct FIDComponent {
    UUID ID;

    FIDComponent() = default;
    FIDComponent(const FIDComponent&) = default;
};
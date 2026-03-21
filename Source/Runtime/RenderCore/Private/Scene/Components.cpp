#include "Components.h"

const FTransformComponent FTransformComponent::Identity =
    FTransformComponent(FVector3::Zero, FQuaternion::Identity(), FVector3::One);
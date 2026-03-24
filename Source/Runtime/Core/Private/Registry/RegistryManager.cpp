#include "RegistryManager.h"

FRegistryManager& FRegistryManager::Get() {
    static FRegistryManager Instance;
    return Instance;
}

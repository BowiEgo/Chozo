#pragma once

#include "AssetRegistry.h"

namespace Chozo {

    class AssetRegistrySerializer
    {
    public:
        AssetRegistrySerializer(const AssetRegistry& context);

        void Serialize(const std::string& filepath);
        void SerializeRuntime(const std::string& filepath);

        AssetRegistry Deserialize(const std::string& filepath);
        AssetRegistry DeserializeRuntime(const std::string& filepath);
    private:
		AssetRegistry m_AssetRegistry;
    };
}
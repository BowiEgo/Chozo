#pragma once

#include "RegistryManager.h"

class FEditorNodeRegistry {
public:
    static FEditorNodeRegistry& Get();

    FTypeInfo RegisterNodeType(const std::string& name, bool bBuiltin = false) {
        auto& manager = FRegistryManager::Get();
        FTypeInfo typeInfo = manager.RegisterType("Node_" + name, bBuiltin, ETypeCategory::Node);

        m_Types.push_back(typeInfo);

        return typeInfo;
    }

    std::vector<FTypeInfo> GetAllTypes() const { return m_Types; }

private:
    FEditorNodeRegistry() {
        // Register Built-in Types
        RegisterNodeType("Root", true);
        RegisterNodeType("Empty", true);
        RegisterNodeType("Regular", true);
    }

    std::vector<FTypeInfo> m_Types;
};

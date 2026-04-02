#pragma once

#include "TypeRegister.h"

class FEditorNodeRegister {
public:
    static FEditorNodeRegister& Get();
    static void Init();

    FTypeInfo RegisterNodeType(const std::string& name, bool bBuiltin = false) {
        auto& manager = FTypeRegister::Get();
        FTypeInfo typeInfo = manager.RegisterType("Node_" + name, bBuiltin, ETypeCategory::Node);

        m_Types.push_back(typeInfo);

        return typeInfo;
    }

    std::vector<FTypeInfo> GetAllTypes() const { return m_Types; }

private:
    FEditorNodeRegister() {
        // Register Built-in Types
        RegisterNodeType("Root", true);
        RegisterNodeType("Empty", true);
        RegisterNodeType("Regular", true);
    }

    std::vector<FTypeInfo> m_Types;
};

#pragma once

#include "TypeRegister.h"

class FLightRegister {
public:
    static FLightRegister& Get();
    static void Init();

    FTypeInfo RegisterLightType(const std::string& name, bool bBuiltin = false) {
        auto& manager = FTypeRegister::Get();
        FTypeInfo typeInfo = manager.RegisterType("Light_" + name, bBuiltin, ETypeCategory::Light);

        m_Types.push_back(typeInfo);

        return typeInfo;
    }

    std::vector<FTypeInfo> GetAllTypes() const { return m_Types; }

    bool IsHDRIBackdropType(uint32_t bit) const {
        auto& manager = FTypeRegister::Get();
        auto HDRIBackdropBit = manager.GetBit("Light_HDRIBackdrop");
        return (bit & HDRIBackdropBit) != 0;
    }

private:
    FLightRegister() {
        // Register Built-in Types
        RegisterLightType("Skylight", true);
        RegisterLightType("Directional", true);
        RegisterLightType("Point", true);
        RegisterLightType("Spot", true);
        RegisterLightType("HDRIBackdrop", true);
    }

    std::vector<FTypeInfo> m_Types;
};

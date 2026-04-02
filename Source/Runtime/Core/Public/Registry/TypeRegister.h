#pragma once

#include "CoreExport.h"
#include "CoreMinimal.h"

enum class ETypeCategory : uint8_t {
    None,
    Node,
    Light,
    Mesh,
};

struct FTypeInfo {
    std::string Name;
    uint32_t Bit;
    bool bBuiltin;

    ETypeCategory Category;

    FTypeInfo() = default;
    FTypeInfo(const std::string& name, uint32_t bit, bool builtin, ETypeCategory category)
        : Name(name), Bit(bit), bBuiltin(builtin), Category(category) {}
};

class CORE_API FTypeRegister {
public:
    static FTypeRegister& Get();

    FTypeInfo RegisterType(const std::string& name, bool bBuiltin = false,
                           ETypeCategory category = ETypeCategory::None) {
        uint32_t bit = 1 << m_NextBit;
        FTypeInfo info(name, bit, bBuiltin, category);

        m_Types.push_back(info);
        m_NameToBit[name] = bit;
        m_BitToInfo[bit] = info;
        m_NextBit++;

        switch (category) {
            case ETypeCategory::Node: m_NodeTypesBit |= bit; break;
            case ETypeCategory::Light: m_LightTypesBit |= bit; break;
            case ETypeCategory::Mesh: m_MeshTypesBit |= bit; break;
            default: break;
        }

        m_AllTypesBit |= bit;

        return info;
    }

    uint32_t GetBit(const std::string& name) const {
        auto it = m_NameToBit.find(name);
        return it != m_NameToBit.end() ? it->second : 0;
    }

    std::string GetName(uint32_t bit) const {
        auto it = m_BitToInfo.find(bit);
        return it != m_BitToInfo.end() ? it->second.Name : "Unknown";
    }

    FTypeInfo* GetTypeInfo(const std::string& name) {
        auto it = m_NameToBit.find(name);
        if (it != m_NameToBit.end()) {
            return &m_BitToInfo[it->second];
        }
        return nullptr;
    }

    FTypeInfo* GetTypeInfo(uint32_t bit) {
        auto it = m_BitToInfo.find(bit);
        return it != m_BitToInfo.end() ? &it->second : nullptr;
    }

    const std::vector<FTypeInfo>& GetAllTypes() const { return m_Types; }

    std::vector<FTypeInfo> GetTypesByCategory(ETypeCategory category) const {
        std::vector<FTypeInfo> result;
        for (const auto& info : m_Types) {
            if (info.Category == category) {
                result.push_back(info);
            }
        }
        return result;
    }

    uint32_t GetNodeTypesMask() const { return m_NodeTypesBit; }
    uint32_t GetLightTypesMask() const { return m_LightTypesBit; }
    uint32_t GetMeshTypesMask() const { return m_MeshTypesBit; }
    uint32_t GetAllTypesMask() const { return m_AllTypesBit; }

    bool IsValidType(uint32_t bit) const { return m_BitToInfo.find(bit) != m_BitToInfo.end(); }

    bool IsNodeType(uint32_t bit) const { return (bit & m_NodeTypesBit) != 0; }
    bool IsLightType(uint32_t bit) const { return (bit & m_LightTypesBit) != 0; }
    bool IsMeshType(uint32_t bit) const { return (bit & m_MeshTypesBit) != 0; }

private:
    FTypeRegister() : m_NextBit(0) {
        // Register Built-in Types
        RegisterType("None", true, ETypeCategory::None);
    }

    uint32_t m_NextBit = 0;
    uint32_t m_AllTypesBit = 0;
    uint32_t m_NodeTypesBit = 0;
    uint32_t m_LightTypesBit = 0;
    uint32_t m_MeshTypesBit = 0;

    std::vector<FTypeInfo> m_Types;
    std::unordered_map<std::string, uint32_t> m_NameToBit;
    std::unordered_map<uint32_t, FTypeInfo> m_BitToInfo;
};

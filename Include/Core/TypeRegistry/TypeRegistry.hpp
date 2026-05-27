#pragma once

#include <Core/TypeRegistry/TypeMask.hpp>

#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace CZ {

// #define TYPE_CATEGORY_LIST \
//     X(TYPE_CATEGORY_NONE) \
//     X(TYPE_CATEGORY_NODE) \
//     X(TYPE_CATEGORY_LIGHT) \
//     X(TYPE_CATEGORY_MESH) \ X(TYPE_CATEGORY_MATERIAL)

// #define X(name) name,
// enum TypeCategory : uint32_t { TYPE_CATEGORY_LIST };
// #undef X

// #define X(name) +1
// constexpr int s_TypeCategoryCount = TYPE_CATEGORY_LIST;
// #undef X

// #define X(name) #name,
// static constexpr std::array<const char*, s_TypeCategoryCount> TypeCategoryStrings = {
//     TYPE_CATEGORY_LIST
// };
// #undef X

namespace TypeCategory {
constexpr uint32_t None     = 0;
constexpr uint32_t Node     = 1 << 0;
constexpr uint32_t Light    = 1 << 1;
constexpr uint32_t Mesh     = 1 << 2;
constexpr uint32_t Material = 1 << 3;
} // namespace TypeCategory

using Type = size_t;

struct TypeInfo {
    std::string Name;
    Type Index;
    bool bBuiltin;
    uint32_t CategoryFlags;

    TypeInfo() = default;
    TypeInfo(const std::string& name, Type index, bool builtin, uint32_t flags)
        : Name(name), Index(index), bBuiltin(builtin), CategoryFlags(flags) {}
};

class TypeRegister {
public:
    static TypeRegister& Get();

    /**
     * @brief  Registers a new type in the global type registry.
     *
     * Assigns a unique index to the type and records its name, built‑in status,
     * and category mask. The category mask can combine multiple flags (e.g.,
     * Node | Mesh) to indicate that the type belongs to several categories
     * simultaneously. The returned index can be used to test category membership
     * via methods like IsNodeType().
     *
     * @param name           Unique human‑readable name for the type.
     * @param bBuiltin       Whether the type is a built‑in engine type (default false).
     * @param categoryFlags  Bitmask of TypeCategory values describing the type's
     *                       category membership (default TYPE_CATEGORY_NONE).
     * @return               The zero‑based index assigned to the type. If the type
     *                       was already registered, its existing index is returned.
     *
     * @note  Thread‑safe: internally protected by a shared mutex.
     * @see   TypeCategory, GetType(), IsNodeType()
     */
    Type RegisterType(const std::string& name, bool bBuiltin = false, uint32_t categoryFlags = 0);

    Type GetType(const std::string& name) const;
    const TypeInfo* GetTypeInfo(Type type) const;

    TypeMask GetMask(Type type) const;
    const TypeMask& GetNodeMask() const { return m_NodeMask; }
    const TypeMask& GetLightMask() const { return m_LightMask; }
    const TypeMask& GetMeshMask() const { return m_MeshMask; }
    const TypeMask& GetMaterialMask() const { return m_MaterialMask; }
    const TypeMask& GetAllMask() const { return m_AllTypesMask; }

    bool IsTypeValid(Type type) { return type != TypeRegister::INVALID_INDEX; }
    bool IsNodeType(Type type) const;
    bool IsLightType(Type type) const;
    bool IsMeshType(Type type) const;
    bool IsMaterialType(Type type) const;

    std::vector<Type> GetNodeAndMeshTypes() const;

    std::string TypeMaskToString(const TypeMask& mask) const;

private:
    static constexpr Type INVALID_INDEX = SIZE_MAX;
    mutable std::shared_mutex m_Mutex;
    std::vector<TypeInfo> m_Types;
    std::unordered_map<std::string, Type> m_NameToType;
    TypeMask m_AllTypesMask;
    TypeMask m_NodeMask, m_LightMask, m_MeshMask, m_MaterialMask;
};

} // namespace CZ

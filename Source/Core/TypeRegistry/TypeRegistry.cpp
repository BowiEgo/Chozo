#include <Core/TypeRegistry/TypeRegistry.hpp>

namespace CZ {

TypeRegister& TypeRegister::Get() {
    static TypeRegister instance;
    return instance;
}

Type TypeRegister::RegisterType(const std::string& name, bool bBuiltin, uint32_t categoryFlags) {
    std::lock_guard lock(m_Mutex);
    if (auto it = m_NameToType.find(name); it != m_NameToType.end()) return it->second;

    size_t index = m_Types.size();
    TypeInfo info{ name, index, bBuiltin, categoryFlags };
    m_Types.push_back(info);
    m_NameToType[name] = index;

    m_AllTypesMask.Set(index);

    if (categoryFlags & TypeCategory::Node) m_NodeMask.Set(index);
    if (categoryFlags & TypeCategory::Light) m_LightMask.Set(index);
    if (categoryFlags & TypeCategory::Mesh) m_MeshMask.Set(index);
    if (categoryFlags & TypeCategory::Material) m_MaterialMask.Set(index);

    return index;
}
Type TypeRegister::GetType(const std::string& name) const {
    std::shared_lock lock(m_Mutex);
    auto it = m_NameToType.find(name);
    return (it != m_NameToType.end()) ? it->second : INVALID_INDEX;
}

const TypeInfo* TypeRegister::GetTypeInfo(Type type) const {
    std::shared_lock lock(m_Mutex);
    return (type < m_Types.size()) ? &m_Types[type] : nullptr;
}

TypeMask TypeRegister::GetMask(size_t index) const {
    TypeMask m;
    m.Set(index);
    return m;
}

bool TypeRegister::IsNodeType(Type type) const {
    std::shared_lock lock(m_Mutex);
    return m_NodeMask.Test(type);
}

bool TypeRegister::IsLightType(Type type) const {
    std::shared_lock lock(m_Mutex);
    return m_MeshMask.Test(type);
}

bool TypeRegister::IsMeshType(Type type) const {
    std::shared_lock lock(m_Mutex);
    return m_MeshMask.Test(type);
}

bool TypeRegister::IsMaterialType(Type type) const {
    std::shared_lock lock(m_Mutex);
    return m_MaterialMask.Test(type);
}

std::vector<Type> TypeRegister::GetNodeAndMeshTypes() const {
    TypeMask combined = m_NodeMask & m_MeshMask;
    std::vector<Type> types;
    for (Type i = 0; i < m_Types.size(); ++i) {
        if (combined.Test(i)) types.push_back(i);
    }
    return types;
}

std::string TypeRegister::TypeMaskToString(const TypeMask& mask) const {
    std::string result;
    for (size_t i = 0; i < m_Types.size(); ++i) {
        if (mask.Test(i)) {
            if (!result.empty()) result += ", ";
            result += m_Types[i].Name;
        }
    }
    return result.empty() ? "None" : result;
}
} // namespace CZ
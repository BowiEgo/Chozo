#include <algorithm>
#include <atomic>
#include <initializer_list>
#include <string>
#include <vector>

#include <cstdint>

#include "EditorNode.hpp"

#include <Runtime/RenderCore/ProceduralMesh/CubeParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/CylinderParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/PlaneParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/SphereParamsObj.hpp>
#include <Runtime/RenderCore/ProceduralMesh/TorusParamsObj.hpp>

std::atomic<uint32_t> EditorNode::s_NextID{ 1 };

EditorNodeRegistry& EditorNodeRegistry::Get() {
    static EditorNodeRegistry instance;
    return instance;
}

bool EditorNodeRegistry::TestSingle(TypeMask mask, const std::string& name) {
    auto type = TypeRegister::Get().GetType(name);
    return mask.Test(type);
}

bool EditorNodeRegistry::Test(TypeMask mask, std::initializer_list<std::string> names) {
    for (const auto& name : names) {
        if (!TestSingle(mask, name)) return false;
    }
    return true;
}

bool EditorNodeRegistry::IsRootType(TypeMask mask) { return TestSingle(mask, "Node_Root"); }

bool EditorNodeRegistry::IsRegularType(TypeMask mask) { return TestSingle(mask, "Node_Regular"); }

bool EditorNodeRegistry::HasMeshType(TypeMask mask) {
    const TypeMask& meshMask = TypeRegister::Get().GetMeshMask();
    return (mask & meshMask).Any();
}

void EditorNodeRegistry::Init() {
    auto nodeType     = TypeRegister::Get().RegisterType("Node_Regular");
    auto rootNodeType = TypeRegister::Get().RegisterType("Node_Root");
}

TypeMask EditorNodeRegistry::GetNodeMask(std::initializer_list<std::string> names) {
    TypeMask result;

    for (const auto& name : names) {
        auto type = TypeRegister::Get().GetType(name);

        if (!TypeRegister::Get().IsTypeValid(type)) {
            CZ_EDITOR_LOG(Warning, "Unknown type name: {}", name);
            continue;
        }

        result.Set(type);
    }
    return result;
}

// ===== Constructor & Destructor =====
EditorNode::EditorNode(const std::string& name, TypeMask typeMask)
    : m_ID(s_NextID.fetch_add(1)), m_Name(name), m_TypeMask(typeMask) {
    if (HasTransform()) {
        SetTransformParams(
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(0.0f, 0.0f, 0.0f)));
        m_DefaultTransformParams = CaptureParams(*m_TransformParams);
    }

    if (HasMesh()) {
        if (EditorNodeRegistry::Test(GetTypeMask(), { "Node_Regular", "ProceduralMesh_Cube" })) {
            SetMeshParams(
                CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, CubeParamsObj, 1.0f, 1.0f, 1.0f, 1, 1, 1));
        } else if (EditorNodeRegistry::Test(GetTypeMask(),
                                            { "Node_Regular", "ProceduralMesh_Sphere" })) {
            SetMeshParams(CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, SphereParamsObj, 1.0f, 32, 16));
            m_DefaultMeshParams = CaptureParams(*m_MeshParams);
        } else if (EditorNodeRegistry::Test(GetTypeMask(),
                                            { "Node_Regular", "ProceduralMesh_Plane" })) {
            SetMeshParams(CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, PlaneParamsObj, 1.0f, 1.0f, 1, 1));
        } else if (EditorNodeRegistry::Test(GetTypeMask(),
                                            { "Node_Regular", "ProceduralMesh_Cylinder" })) {
            SetMeshParams(
                CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, CylinderParamsObj, 1.0f, 1.0f, 32, 1));
        } else if (EditorNodeRegistry::Test(GetTypeMask(),
                                            { "Node_Regular", "ProceduralMesh_Torus" })) {
            SetMeshParams(CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, TorusParamsObj, 1.0f, 0.35f, 32, 16));
        }
    }
}

EditorNode::~EditorNode() {
    // Parameters are owned by Scope members and released automatically.
    while (!m_Children.empty()) {
        Delete(m_Children.back());
    }

    if (m_Parent) {
        m_Parent->RemoveChild(this);
    }
}

// ===== Hierarchy Management =====
void EditorNode::AddChild(EditorNode* child) {
    if (!child) {
        CZ_EDITOR_LOG(Warning, "Attempted to add null child to node '{}' with ID {}", m_Name, m_ID);
        return;
    }

    if (child == this) {
        CZ_EDITOR_LOG(Warning, "Attempted to add node to itself: '{}' with ID {}", m_Name, m_ID);
        return;
    }

    // Check if already a child (prevent duplicate addition)
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        CZ_EDITOR_LOG(Warning, "Child '{}' already exists in node '{}'", child->GetName(), m_Name);
        return;
    }

    // If child already has a parent, remove from old parent first
    if (child->m_Parent) {
        child->m_Parent->RemoveChild(child);
    }

    child->m_Parent = this;
    m_Children.push_back(child);

    MarkDirty();
    child->MarkDirty();
}

void EditorNode::RemoveChild(EditorNode* child) {
    if (!child) {
        CZ_EDITOR_LOG(Warning, "Attempted to remove null child from node '{}' with ID {}", m_Name,
                      m_ID);
        return;
    }

    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it == m_Children.end()) {
        CZ_EDITOR_LOG(Warning, "Child '{}' not found in node '{}'", child->GetName(), m_Name);
        return;
    }

    child->m_Parent = nullptr;
    m_Children.erase(it);

    MarkDirty();
    child->MarkDirty();
}

void EditorNode::SetParent(EditorNode* parent) {
    if (m_Parent == parent) {
        return;
    }

    // Check for circular reference (cannot set ancestor as child)
    if (parent && IsAncestorOf(parent)) {
        CZ_EDITOR_LOG(Warning, "Cannot set parent '{}' to node '{}' (would create cycle)",
                      parent->GetName(), m_Name);
        return;
    }

    if (m_Parent) {
        m_Parent->RemoveChild(this);
    }

    if (parent) {
        parent->AddChild(this);
    } else {
        m_Parent = nullptr;
        MarkDirty();
    }
}

// ===== Utility =====
bool EditorNode::IsAncestorOf(EditorNode* node) const {
    if (!node) return false;

    EditorNode* current = node->m_Parent;
    while (current) {
        if (current == this) {
            return true;
        }
        current = current->m_Parent;
    }
    return false;
}

bool EditorNode::IsDescendantOf(EditorNode* node) const {
    if (!node) return false;
    return node->IsAncestorOf(const_cast<EditorNode*>(this));
}

EditorNode* EditorNode::FindChild(const std::string& name) const {
    for (auto* child : m_Children) {
        if (child->GetName() == name) {
            return child;
        }
        auto* found = child->FindChild(name);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

std::vector<EditorNode*> EditorNode::GetChildrenRecursive() const {
    std::vector<EditorNode*> result;
    for (auto* child : m_Children) {
        result.push_back(child);
        auto grandchildren = child->GetChildrenRecursive();
        result.insert(result.end(), grandchildren.begin(), grandchildren.end());
    }
    return result;
}

int EditorNode::GetDepth() const {
    int depth           = 0;
    EditorNode* current = m_Parent;
    while (current) {
        depth++;
        current = current->m_Parent;
    }
    return depth;
}

// ===== Serialization =====
//     ar << m_Name;
//     ar << static_cast<int32_t>(m_Type);
//     ar << m_Transform;

//     ar << childCount;

//     ar >> m_Name;
//     ar >> type;
//     ar >> m_Transform;

//     ar >> childCount;

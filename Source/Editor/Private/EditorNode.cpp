#include "EditorNode.h"

DEFINE_LOG_CATEGORY(LogEditorNode);

// ===== Constructor & Destructor =====
FEditorNode::FEditorNode(const std::string& name, ENodeType type) : m_Name(name), m_Type(type) {}

FEditorNode::~FEditorNode() {
    for (auto* child : m_Children) {
        delete child;
    }
    m_Children.clear();

    if (m_Parent) {
        m_Parent->RemoveChild(this);
    }
}

// ===== Hierarchy Management =====
void FEditorNode::AddChild(FEditorNode* child) {
    if (!child) {
        CZ_LOG(LogEditorNode, Warning, "Attempted to add null child to node '{}'", m_Name);
        return;
    }

    if (child == this) {
        CZ_LOG(LogEditorNode, Warning, "Attempted to add node to itself: '{}'", m_Name);
        return;
    }

    // Check if already a child (prevent duplicate addition)
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        CZ_LOG(LogEditorNode, Warning, "Child '{}' already exists in node '{}'", child->GetName(),
               m_Name);
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

    CZ_LOG(LogEditorNode, Trace, "Added child '{}' to node '{}'", child->GetName(), m_Name);
}

void FEditorNode::RemoveChild(FEditorNode* child) {
    if (!child) {
        CZ_LOG(LogEditorNode, Warning, "Attempted to remove null child from node '{}'", m_Name);
        return;
    }

    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it == m_Children.end()) {
        CZ_LOG(LogEditorNode, Warning, "Child '{}' not found in node '{}'", child->GetName(),
               m_Name);
        return;
    }

    child->m_Parent = nullptr;
    m_Children.erase(it);

    MarkDirty();
    child->MarkDirty();

    CZ_LOG(LogEditorNode, Trace, "Removed child '{}' from node '{}'", child->GetName(), m_Name);
}

void FEditorNode::SetParent(FEditorNode* parent) {
    if (m_Parent == parent) {
        return;
    }

    // Check for circular reference (cannot set ancestor as child)
    if (parent && IsAncestorOf(parent)) {
        CZ_LOG(LogEditorNode, Warning, "Cannot set parent '{}' to node '{}' (would create cycle)",
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
bool FEditorNode::IsAncestorOf(FEditorNode* node) const {
    if (!node) return false;

    FEditorNode* current = node->m_Parent;
    while (current) {
        if (current == this) {
            return true;
        }
        current = current->m_Parent;
    }
    return false;
}

bool FEditorNode::IsDescendantOf(FEditorNode* node) const {
    if (!node) return false;
    return node->IsAncestorOf(const_cast<FEditorNode*>(this));
}

FEditorNode* FEditorNode::FindChild(const std::string& name) const {
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

std::vector<FEditorNode*> FEditorNode::GetChildrenRecursive() const {
    std::vector<FEditorNode*> result;
    for (auto* child : m_Children) {
        result.push_back(child);
        auto grandchildren = child->GetChildrenRecursive();
        result.insert(result.end(), grandchildren.begin(), grandchildren.end());
    }
    return result;
}

int FEditorNode::GetDepth() const {
    int depth = 0;
    FEditorNode* current = m_Parent;
    while (current) {
        depth++;
        current = current->m_Parent;
    }
    return depth;
}

// ===== Component Management =====
// void FEditorNode::SetMesh(const FMeshComponent& mesh) {
//     m_Mesh = mesh;
//     m_HasMesh = true;
//     MarkDirty();
// }

// void FEditorNode::ClearMesh() {
//     m_HasMesh = false;
//     MarkDirty();
// }

// void FEditorNode::SetMaterial(const FMaterialComponent& material) {
//     m_Material = material;
//     m_HasMaterial = true;
//     MarkDirty();
// }

// void FEditorNode::ClearMaterial() {
//     m_HasMaterial = false;
//     MarkDirty();
// }

// ===== Serialization =====
// void FEditorNode::Serialize(FArchive& ar) {
//     ar << m_Name;
//     ar << static_cast<int32_t>(m_Type);
//     ar << m_Transform;

//     uint32_t childCount = static_cast<uint32_t>(m_Children.size());
//     ar << childCount;

//     for (auto* child : m_Children) {
//         child->Serialize(ar);
//     }
// }

// void FEditorNode::Deserialize(FArchive& ar) {
//     ar >> m_Name;
//     int32_t type;
//     ar >> type;
//     m_Type = static_cast<ENodeType>(type);
//     ar >> m_Transform;

//     uint32_t childCount;
//     ar >> childCount;
//     for (uint32_t i = 0; i < childCount; i++) {
//         auto* child = new FEditorNode();
//         child->Deserialize(ar);
//         AddChild(child);
//     }
// }

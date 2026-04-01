#pragma once

#include "Components.h"
#include "EditorNodeRegistry.h"
#include "Entity.h"
#include "Matrix4.h"
#include "MeshRegistry.h"
#include "Params.h"
#include "Vector3.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorNode, Info);

// enum class ENodeType : uint32_t {
//     None = 0,
//     Root = 1 << 0,     // 1
//     Sphere = 1 << 1,   // 2
//     Cube = 1 << 2,     // 4
//     Cylinder = 1 << 3, // 8
//     Plane = 1 << 4,    // 16
//     Empty = 1 << 5,    // 32

//     // Convenience combinations
//     MeshTypes = Sphere | Cube | Cylinder | Plane,
//     All = Root | MeshTypes | Empty
// };

// // Bitwise operators for ENodeType
// inline ENodeType operator|(ENodeType a, ENodeType b) {
//     return static_cast<ENodeType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
// }

// inline ENodeType operator&(ENodeType a, ENodeType b) {
//     return static_cast<ENodeType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
// }

// inline ENodeType operator^(ENodeType a, ENodeType b) {
//     return static_cast<ENodeType>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
// }

// inline ENodeType operator~(ENodeType a) {
//     return static_cast<ENodeType>(~static_cast<uint32_t>(a));
// }

// inline ENodeType& operator|=(ENodeType& a, ENodeType b) {
//     a = a | b;
//     return a;
// }

// inline ENodeType& operator&=(ENodeType& a, ENodeType b) {
//     a = a & b;
//     return a;
// }

// inline bool HasFlag(ENodeType value, ENodeType flag) {
//     return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
// }

class EDITOR_API FEditorNode {
public:
    FEditorNode() = default;
    explicit FEditorNode(const std::string& name, uint32_t type);
    ~FEditorNode();

    // ===== Identity =====
    uint32_t GetID() const { return m_ID; }
    void SetName(const std::string& name) {
        m_Name = name;
        MarkDirty();
    }
    const std::string& GetName() const { return m_Name; }

    // ===== Type Management =====
    uint32_t GetType() const { return m_Type; }
    // void SetType(uint32_t type) {
    //     m_Type = type;
    //     MarkDirty();
    // }
    // void AddType(uint32_t type) {
    //     m_Type = m_Type | type;
    //     MarkDirty();
    // }
    // void RemoveType(uint32_t type) {
    //     m_Type = m_Type & ~type;
    //     MarkDirty();
    // }
    // bool HasType(uint32_t type) const { return HasFlag(m_Type, type); }

    // ===== Hierarchy =====
    void AddChild(FEditorNode* child);
    void RemoveChild(FEditorNode* child);
    void SetParent(FEditorNode* parent);

    FEditorNode* GetParent() const { return m_Parent; }
    const std::vector<FEditorNode*>& GetChildren() const { return m_Children; }

    // Hierarchy queries
    bool IsRoot() const { return m_Parent == nullptr; }
    bool IsLeaf() const { return m_Children.empty(); }
    bool IsAncestorOf(FEditorNode* node) const;
    bool IsDescendantOf(FEditorNode* node) const;
    FEditorNode* FindChild(const std::string& name) const;
    std::vector<FEditorNode*> GetChildrenRecursive() const;
    int GetDepth() const;

    // ===== State =====
    void Open() { m_bOpen = true; }
    void Close() { m_bOpen = false; }
    void ToggleOpen() { m_bOpen = !m_bOpen; }
    bool IsOpen() const { return m_bOpen; }

    void MarkDirty() { m_bDirty = true; }
    void ClearDirty() { m_bDirty = false; }
    bool IsDirty() const { return m_bDirty; }

    // ===== Tags =====
    bool HasTags() const { return !m_Tags.empty(); }
    const std::unordered_set<std::string>& GetTags() const { return m_Tags; }
    void AddTag(const std::string& tag) {
        m_Tags.insert(tag);
        MarkDirty();
    }
    void RemoveTag(const std::string& tag) {
        m_Tags.erase(tag);
        MarkDirty();
    }
    bool HasTag(const std::string& tag) const { return m_Tags.find(tag) != m_Tags.end(); }
    void ClearTags() {
        m_Tags.clear();
        MarkDirty();
    }

    // ===== Transform =====
    bool HasTransform() const {
        auto regNodeBit = FRegistryManager::Get().GetBit("Node_Regular");
        return (m_Type & regNodeBit) != 0;
    }
    void SetTransformParams(const FTransformParams& params) {
        m_TransformParams = params;
        MarkDirty();
    }
    const FTransformParams* GetTransformParams() const { return &m_TransformParams; }
    FTransformParams* GetTransformParams() { return &m_TransformParams; }

    // ===== Mesh =====
    bool HasMesh() const { return FRegistryManager::Get().IsMeshType(m_Type); }
    void SetMeshParams(const FMeshParams& params) {
        m_MeshParams = params;
        MarkDirty();
    }
    void SetMeshParams(const std::string& typeName) {
        m_MeshParams = FMeshRegistry::Get().CreateParams(typeName);
        MarkDirty();
    }
    const FMeshParams* GetMeshParams() const { return &m_MeshParams; }
    FMeshParams* GetMeshParams() { return &m_MeshParams; }

    // ===== Material =====

    // ===== Serialization =====
    // void Serialize(FArchive& ar);
    // void Deserialize(FArchive& ar);

private:
    static std::atomic<uint32_t> s_NextID;
    uint32_t m_ID;

    std::string m_Name;
    uint32_t m_Type;
    std::unordered_set<std::string> m_Tags;

    // Hierarchy
    FEditorNode* m_Parent = nullptr;
    std::vector<FEditorNode*> m_Children;

    // Components
    FTransformParams m_TransformParams;
    FMeshParams m_MeshParams;

    // State
    bool m_bDirty = false;
    bool m_bOpen = false;
};

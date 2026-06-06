#pragma once

// #include "Components.h"
// #include "EditorNodeRegister.h"
// #include "Entity.h"
// #include "LightRegister.h"
// #include "MeshRegister.h"
// #include "Params.h"
// #include "Vector3.h"
// #include "Matrix4.h"
#include <Core/TypeRegistry/TypeRegistry.hpp>
#include <Runtime/RenderCore/Components/TransformParams.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>

using namespace CZ;

#define GET_NODE_MASK(...) EditorNodeRegistry::Get().GetNodeMask({ __VA_ARGS__ })

using NodeID = uint32_t;

class EditorNodeRegistry {
public:
    static EditorNodeRegistry& Get();

    static bool TestSingle(TypeMask mask, const std::string& name);
    static bool Test(TypeMask mask, std::initializer_list<std::string> names);
    static bool IsRootType(TypeMask mask);
    static bool IsRegularType(TypeMask mask);
    static bool HasMeshType(TypeMask mask);

    void Init();

    TypeMask GetNodeMask(std::initializer_list<std::string> names);
};

class EditorNode {
public:
    EditorNode() = default;
    explicit EditorNode(const std::string& name, TypeMask typeMask);
    ~EditorNode();

    // ===== Identity =====
    NodeID GetID() const { return m_ID; }
    void SetName(const std::string& name) {
        m_Name = name;
        MarkDirty();
    }
    const std::string& GetName() const { return m_Name; }

    // ===== TypeMask Management =====
    TypeMask GetTypeMask() const { return m_TypeMask; }

    // ===== Hierarchy =====
    void AddChild(EditorNode* child);
    void RemoveChild(EditorNode* child);
    void SetParent(EditorNode* parent);

    EditorNode* GetParent() const { return m_Parent; }
    const std::vector<EditorNode*>& GetChildren() const { return m_Children; }

    // Hierarchy queries
    bool IsRoot() const { return m_Parent == nullptr; }
    bool IsLeaf() const { return m_Children.empty(); }
    bool IsAncestorOf(EditorNode* node) const;
    bool IsDescendantOf(EditorNode* node) const;
    EditorNode* FindChild(const std::string& name) const;
    std::vector<EditorNode*> GetChildrenRecursive() const;
    int GetDepth() const;

    // ===== State =====
    void Open() { m_IsOpen = true; }
    void Close() { m_IsOpen = false; }
    void ToggleOpen() { m_IsOpen = !m_IsOpen; }
    bool IsOpen() const { return m_IsOpen; }

    void MarkDirty() { m_IsDirty = true; }
    void ClearDirty() { m_IsDirty = false; }
    bool IsDirty() const { return m_IsDirty; }

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
    bool HasTransform() const { return EditorNodeRegistry::IsRegularType(m_TypeMask); }
    void SetTransformParams(const TransformParams params) {
        m_TransformParams = params;
        MarkDirty();
    }
    const TransformParams GetTransformParams() const { return m_TransformParams; }
    TransformParams GetTransformParams() { return m_TransformParams; }

    // ===== HDRIBackdrop =====
    // bool HasHDRIBackdrop() const {
    //     bool isHDRIBackdrop = FLightRegister::Get().IsHDRIBackdropType(m_TypeMask);
    //     return isHDRIBackdrop;
    // }
    // void SetHDRIBackdropParams(const HDRIBackdropParams& params) {
    //     m_HDRIBackdropParams = params;
    //     MarkDirty();
    // }
    // const HDRIBackdropParams* GetHDRIBackdropParams() const { return &m_HDRIBackdropParams; }
    // HDRIBackdropParams* GetHDRIBackdropParams() { return &m_HDRIBackdropParams; }

    // // ===== Mesh =====
    bool HasMesh() const { return EditorNodeRegistry::HasMeshType(m_TypeMask); }
    void SetMeshParams(const MeshParams& props) {
        m_MeshParams.Destroy();
        m_MeshParams = props;
        MarkDirty();
    }
    // void SetMeshParams(const std::string& typeName) {
    //     m_MeshParams = MeshRegister::Get().CreateParams(typeName);
    //     MarkDirty();
    // }
    const MeshParams GetMeshParams() const { return m_MeshParams; }
    MeshParams GetMeshParams() { return m_MeshParams; }

    // ===== Material =====

    // ===== Serialization =====
    // void Serialize(FArchive& ar);
    // void Deserialize(FArchive& ar);

private:
    static std::atomic<NodeID> s_NextID;
    NodeID m_ID;

    std::string m_Name;
    TypeMask m_TypeMask;
    std::unordered_set<std::string> m_Tags;

    // Hierarchy
    EditorNode* m_Parent = nullptr;
    std::vector<EditorNode*> m_Children;

    // Components
    TransformParams m_TransformParams;
    // HDRIBackdropParams m_HDRIBackdropParams;
    MeshParams m_MeshParams;

    // State
    bool m_IsDirty = false;
    bool m_IsOpen  = false;
};

#pragma once

#include "Components.h"
#include "Entity.h"
#include "Matrix4.h"
#include "Vector3.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorNode, Info);

enum class ENodeType { Root, Sphere, Cube, Cylinder, Plane, Empty };

class EDITOR_API FEditorNode {
public:
    FEditorNode() = default;
    explicit FEditorNode(const std::string& name, ENodeType type = ENodeType::Empty);
    ~FEditorNode();

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
    void MarkDirty() { m_Dirty = true; }
    void ClearDirty() { m_Dirty = false; }
    bool IsDirty() const { return m_Dirty; }

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

    // ===== Properties =====
    void SetName(const std::string& name) {
        m_Name = name;
        MarkDirty();
    }
    const std::string& GetName() const { return m_Name; }

    ENodeType GetType() const { return m_Type; }
    void SetType(ENodeType type) {
        m_Type = type;
        MarkDirty();
    }

    // ===== Transform =====
    FTransformComponent& GetTransform() { return m_Transform; }
    const FTransformComponent& GetTransform() const { return m_Transform; }
    void SetTransform(const FTransformComponent& transform) {
        m_Transform = transform;
        MarkDirty();
    }

    // // ===== Mesh =====
    // bool HasMesh() const { return m_HasMesh; }
    // FMeshComponent& GetMesh() { return m_Mesh; }
    // const FMeshComponent& GetMesh() const { return m_Mesh; }
    // void SetMesh(const FMeshComponent& mesh);
    // void ClearMesh();

    // // ===== Material =====
    // bool HasMaterial() const { return m_HasMaterial; }
    // FMaterialComponent& GetMaterial() { return m_Material; }
    // const FMaterialComponent& GetMaterial() const { return m_Material; }
    // void SetMaterial(const FMaterialComponent& material);
    // void ClearMaterial();

    // ===== Serialization =====
    // void Serialize(FArchive& ar);
    // void Deserialize(FArchive& ar);

private:
    std::string m_Name;
    ENodeType m_Type = ENodeType::Empty;
    std::unordered_set<std::string> m_Tags;

    // Hierarchy
    FEditorNode* m_Parent = nullptr;
    std::vector<FEditorNode*> m_Children;

    // Components
    FTransformComponent m_Transform;
    // FMeshComponent m_Mesh;
    // FMaterialComponent m_Material;
    bool m_HasMesh = false;
    bool m_HasMaterial = false;

    // State
    bool m_Dirty = false;
};

#pragma once

#include "EditorNode.hpp"

#include <Core/Header/Scope.h>
#include <unordered_map>

enum class EditorNodeEventType { Created, Deleted, Renamed, Moved, Selected, DirtyChanged };

class NodeEvent {
public:
    NodeEvent(EditorNodeEventType type, EditorNode* node) : m_Type(type), m_Node(node) {}

    NodeEvent(EditorNodeEventType type, EditorNode* node, const std::string& oldName)
        : m_Type(type), m_Node(node), m_OldName(oldName) {}

    NodeEvent(EditorNodeEventType type, EditorNode* node, EditorNode* oldParent)
        : m_Type(type), m_Node(node), m_OldParent(oldParent) {}

    EditorNodeEventType GetType() const { return m_Type; }
    EditorNode* GetNode() const { return m_Node; }
    const std::string& GetOldName() const { return m_OldName; }
    EditorNode* GetOldParent() const { return m_OldParent; }

private:
    EditorNodeEventType m_Type;
    EditorNode* m_Node = nullptr;
    std::string m_OldName;
    EditorNode* m_OldParent = nullptr;
};

using NodeEventCallback = std::function<void(const NodeEvent&)>;

struct CallbackEntry {
    CallbackHandle Handle;
    NodeEventCallback Callback;
};

class EditorNodeTree {
public:
    EditorNodeTree() {};
    ~EditorNodeTree();

    void Init();

    // ===== Node Lifecycle =====
    EditorNode* CreateNode(const std::string& name, TypeMask typeMask,
                           EditorNode* parent = nullptr);
    void DeleteNode(EditorNode* node);
    void DuplicateNode(EditorNode* node);

    // ===== Node Attributes =====
    void RenameNode(EditorNode* node, const std::string& newName);
    // void SetNodeType(EditorNode* node, Type type);
    void SetNodeParent(EditorNode* node, EditorNode* newParent);

    // ===== Node Query =====
    EditorNode* GetRoot() const { return m_RootNode; }
    EditorNode* GetSelectedNode() const { return m_SelectedNode; }
    std::vector<EditorNode*> GetSelectedNodes() const { return m_SelectedNodes; }

    EditorNode* QueryNodeByID(NodeID id) const;
    EditorNode* QueryNodeByName(const std::string& name) const;
    std::vector<EditorNode*> QueryNodesByType(Type type) const;
    std::vector<EditorNode*> QueryNodesByTypeMask(const TypeMask& queryMask) const;
    std::vector<EditorNode*> QueryNodesByTypeNames(std::initializer_list<std::string> names) const;
    std::vector<EditorNode*> QueryNodesByTag(const std::string& tag) const;

    void SelectNode(EditorNode* node, bool addToSelection = false);
    void DeselectNode(EditorNode* node);
    void ClearSelection();
    bool IsSelected(EditorNode* node) const;

    // ===== Recursive =====
    void ForEachNode(std::function<void(EditorNode*)> callback);
    void ForEachNodeRecursive(std::function<void(EditorNode*)> callback);

    // ===== Event System =====
    CallbackHandle RegisterEventCallback(NodeEventCallback callback);
    void UnregisterEventCallback(CallbackHandle handle);
    void UnregisterAllCallbacks();

    // ===== Serializing =====
    void Serialize(const std::string& path);
    void Deserialize(const std::string& path);

    // ===== Validation =====
    bool ValidateTree() const;
    void RepairTree();

private:
    void UpdateNodeCache(EditorNode* node);
    void RemoveFromCache(EditorNode* node);
    void NotifyEvent(const NodeEvent& event);

    void OnNodeCreated(EditorNode* node);
    void OnNodeDeleted(EditorNode* node);
    void OnNodeRenamed(EditorNode* node, const std::string& oldName);
    void OnNodeParentChanged(EditorNode* node, EditorNode* oldParent);
    void OnNodeSelected(EditorNode* node);

private:
    EditorNode* m_RootNode     = nullptr;
    EditorNode* m_SelectedNode = nullptr;
    std::vector<EditorNode*> m_SelectedNodes;

    // Cache（Quick Finding）
    std::unordered_map<uint32_t, EditorNode*> m_IdToNodeMap;
    std::unordered_map<std::string, std::vector<EditorNode*>> m_NameToNodesMap;
    std::unordered_map<Type, std::vector<EditorNode*>> m_TypeToNodesMap;
    std::unordered_map<std::string, std::vector<EditorNode*>> m_TagToNodesMap;

    // Event Callbacks
    std::vector<CallbackEntry> m_EventCallbacks;
    CallbackHandle m_NextHandle = 1;

    // Dirt Flag
    bool m_IsDirty = false;
};
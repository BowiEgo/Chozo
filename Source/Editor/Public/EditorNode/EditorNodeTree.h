#pragma once

#include "EditorNode.h"

#include <functional>
#include <memory>
#include <unordered_map>

enum class ENodeEventType { Created, Deleted, Renamed, Moved, Selected, DirtyChanged };

class FNodeEvent {
public:
    FNodeEvent(ENodeEventType type, FEditorNode* node) : m_Type(type), m_Node(node) {}

    FNodeEvent(ENodeEventType type, FEditorNode* node, const std::string& oldName)
        : m_Type(type), m_Node(node), m_OldName(oldName) {}

    FNodeEvent(ENodeEventType type, FEditorNode* node, FEditorNode* oldParent)
        : m_Type(type), m_Node(node), m_OldParent(oldParent) {}

    ENodeEventType GetType() const { return m_Type; }
    FEditorNode* GetNode() const { return m_Node; }
    const std::string& GetOldName() const { return m_OldName; }
    FEditorNode* GetOldParent() const { return m_OldParent; }

private:
    ENodeEventType m_Type;
    FEditorNode* m_Node = nullptr;
    std::string m_OldName;
    FEditorNode* m_OldParent = nullptr;
};

using FNodeEventCallback = std::function<void(const FNodeEvent&)>;

struct FCallbackEntry {
    CallbackHandle Handle;
    FNodeEventCallback Callback;
};

class CEditorNodeTree {
public:
    CEditorNodeTree();
    ~CEditorNodeTree();

    // ===== Node Lifecycle =====
    FEditorNode* CreateNode(const std::string& name, uint32_t typeBit,
                            FEditorNode* parent = nullptr);
    void DeleteNode(FEditorNode* node);
    void DuplicateNode(FEditorNode* node);

    // ===== Node Attributes =====
    void RenameNode(FEditorNode* node, const std::string& newName);
    // void SetNodeType(FEditorNode* node, uint32_t typeBit);
    void SetNodeParent(FEditorNode* node, FEditorNode* newParent);

    // ===== Node Query =====
    FEditorNode* GetRoot() const { return m_RootNode; }
    FEditorNode* GetSelectedNode() const { return m_SelectedNode; }
    std::vector<FEditorNode*> GetSelectedNodes() const { return m_SelectedNodes; }

    FEditorNode* FindNodeByID(uint32_t id) const;
    FEditorNode* FindNodeByName(const std::string& name) const;
    std::vector<FEditorNode*> FindNodesByType(uint32_t typeBit) const;
    std::vector<FEditorNode*> FindNodesByTag(const std::string& tag) const;

    void SelectNode(FEditorNode* node, bool addToSelection = false);
    void DeselectNode(FEditorNode* node);
    void ClearSelection();
    bool IsSelected(FEditorNode* node) const;

    // ===== Recursive =====
    void ForEachNode(std::function<void(FEditorNode*)> callback);
    void ForEachNodeRecursive(std::function<void(FEditorNode*)> callback);

    // ===== Event System =====
    CallbackHandle RegisterEventCallback(FNodeEventCallback callback);
    void UnregisterEventCallback(CallbackHandle handle);
    void UnregisterAllCallbacks();

    // ===== Serializing =====
    void Serialize(const std::string& path);
    void Deserialize(const std::string& path);

    // ===== Validation =====
    bool ValidateTree() const;
    void RepairTree();

private:
    void UpdateNodeCache(FEditorNode* node);
    void RemoveFromCache(FEditorNode* node);
    void NotifyEvent(const FNodeEvent& event);

    void OnNodeCreated(FEditorNode* node);
    void OnNodeDeleted(FEditorNode* node);
    void OnNodeRenamed(FEditorNode* node, const std::string& oldName);
    void OnNodeParentChanged(FEditorNode* node, FEditorNode* oldParent);
    void OnNodeSelected(FEditorNode* node);

private:
    FEditorNode* m_RootNode = nullptr;
    FEditorNode* m_SelectedNode = nullptr;
    std::vector<FEditorNode*> m_SelectedNodes;

    // Cache（Quick Finding）
    std::unordered_map<uint32_t, FEditorNode*> m_IdToNodeMap;
    std::unordered_map<std::string, std::vector<FEditorNode*>> m_NameToNodeMap;
    std::unordered_map<uint32_t, std::vector<FEditorNode*>> m_TypeToNodeMap;
    std::unordered_map<std::string, std::vector<FEditorNode*>> m_TagToNodeMap;

    // Event Callbacks
    std::vector<FCallbackEntry> m_EventCallbacks;
    CallbackHandle m_NextHandle = 1;

    // Dirt Flag
    bool m_IsDirty = false;
};
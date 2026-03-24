#include "EditorNodeTree.h"

#include "EditorNodeRegistry.h"
#include "RegistryManager.h"

#include <algorithm>

static FEditorNode* CreateDemoTree(CEditorNodeTree* tree) {
    const std::vector<std::string> fruitNames = { "Apple",      "Banana",    "Cherry", "Kiwi",
                                                  "Mango",      "Orange",    "Pear",   "Pineapple",
                                                  "Strawberry", "Watermelon" };

    const int multiplier = 2;
    auto nodeBit = FRegistryManager::Get().GetBit("Node_Regular");
    auto rootBit = FRegistryManager::Get().GetBit("Node_Root");
    auto sphereBit = FRegistryManager::Get().GetBit("Mesh_Sphere");
    auto cubeBit = FRegistryManager::Get().GetBit("Mesh_Cube");

    FEditorNode* root = new FEditorNode("Root", rootBit);

    for (size_t i = 0; i < fruitNames.size() * multiplier; ++i) {
        size_t fruitIndex = i / multiplier;
        int instance = i % multiplier;

        std::string nodeName = fruitNames[fruitIndex] + " " + std::to_string(instance);

        FEditorNode* fruitNode = tree->CreateNode(nodeName, nodeBit |= sphereBit, root);

        int childCount = static_cast<int>(nodeName.length());

        for (int j = 0; j < childCount; ++j) {
            std::string childName = "Child " + std::to_string(j);
            FEditorNode* childNode = tree->CreateNode(childName, nodeBit |= cubeBit, fruitNode);
        }
    }

    return root;
}

CEditorNodeTree::CEditorNodeTree() {
    auto nodeReg = FEditorNodeRegistry::Get();
    auto rootBit = FRegistryManager::Get().GetBit("Node_Root");

    m_RootNode = new FEditorNode("Root", rootBit);
    // m_RootNode = CreateDemoTree(this);

    UpdateNodeCache(m_RootNode);
}

CEditorNodeTree::~CEditorNodeTree() {
    delete m_RootNode;
    m_IdToNodeMap.clear();
    m_NameToNodeMap.clear();
    m_TypeToNodeMap.clear();
    m_TagToNodeMap.clear();
}

FEditorNode* CEditorNodeTree::CreateNode(const std::string& name, uint32_t typeBit,
                                         FEditorNode* parent) {
    if (!parent) {
        parent = m_RootNode;
    }

    FEditorNode* node = new FEditorNode(name, typeBit);
    parent->AddChild(node);

    UpdateNodeCache(node);
    OnNodeCreated(node);

    return node;
}

void CEditorNodeTree::DeleteNode(FEditorNode* node) {
    if (!node || node == m_RootNode) return;

    FEditorNode* parent = node->GetParent();
    if (parent) {
        parent->RemoveChild(node);
    }

    if (m_SelectedNode == node) {
        m_SelectedNode = nullptr;
    }
    auto it = std::find(m_SelectedNodes.begin(), m_SelectedNodes.end(), node);
    if (it != m_SelectedNodes.end()) {
        m_SelectedNodes.erase(it);
    }

    RemoveFromCache(node);
    OnNodeDeleted(node);

    delete node;
    m_IsDirty = true;
}

void CEditorNodeTree::RenameNode(FEditorNode* node, const std::string& newName) {
    if (!node) return;

    std::string oldName = node->GetName();
    if (oldName == newName) return;

    node->SetName(newName);

    // Update Cache
    auto& vec = m_NameToNodeMap[oldName];
    vec.erase(std::remove(vec.begin(), vec.end(), node), vec.end());
    if (vec.empty()) {
        m_NameToNodeMap.erase(oldName);
    }
    m_NameToNodeMap[newName].push_back(node);

    OnNodeRenamed(node, oldName);
    m_IsDirty = true;
}

void CEditorNodeTree::SetNodeParent(FEditorNode* node, FEditorNode* newParent) {
    if (!node || node == m_RootNode) return;
    if (newParent == node->GetParent()) return;

    FEditorNode* oldParent = node->GetParent();
    node->SetParent(newParent);

    OnNodeParentChanged(node, oldParent);
    m_IsDirty = true;
}

FEditorNode* CEditorNodeTree::FindNodeByID(uint32_t id) const {
    auto it = m_IdToNodeMap.find(id);
    return it != m_IdToNodeMap.end() ? it->second : nullptr;
}

FEditorNode* CEditorNodeTree::FindNodeByName(const std::string& name) const {
    auto it = m_NameToNodeMap.find(name);
    return it != m_NameToNodeMap.end() && !it->second.empty() ? it->second[0] : nullptr;
}

std::vector<FEditorNode*> CEditorNodeTree::FindNodesByType(uint32_t typeBit) const {
    auto it = m_TypeToNodeMap.find(typeBit);
    return it != m_TypeToNodeMap.end() ? it->second : std::vector<FEditorNode*>();
}

std::vector<FEditorNode*> CEditorNodeTree::FindNodesByTag(const std::string& tag) const {
    auto it = m_TagToNodeMap.find(tag);
    return it != m_TagToNodeMap.end() ? it->second : std::vector<FEditorNode*>();
}

void CEditorNodeTree::SelectNode(FEditorNode* node, bool addToSelection) {
    if (node &&
        std::find(m_SelectedNodes.begin(), m_SelectedNodes.end(), node) == m_SelectedNodes.end()) {
        if (!addToSelection) {
            ClearSelection();
        }

        m_SelectedNodes.push_back(node);
        m_SelectedNode = node;

        OnNodeSelected(node);
    }
}

void CEditorNodeTree::ClearSelection() {
    m_SelectedNodes.clear();
    m_SelectedNode = nullptr;
}

bool CEditorNodeTree::IsSelected(FEditorNode* node) const {
    return std::find(m_SelectedNodes.begin(), m_SelectedNodes.end(), node) != m_SelectedNodes.end();
}

void CEditorNodeTree::ForEachNodeRecursive(std::function<void(FEditorNode*)> callback) {
    std::function<void(FEditorNode*)> traverse = [&](FEditorNode* node) {
        callback(node);
        for (auto* child : node->GetChildren()) {
            traverse(child);
        }
    };
    traverse(m_RootNode);
}

void CEditorNodeTree::UpdateNodeCache(FEditorNode* node) {
    m_IdToNodeMap[node->GetID()] = node;
    m_NameToNodeMap[node->GetName()].push_back(node);
    m_TypeToNodeMap[node->GetType()].push_back(node);

    for (const auto& tag : node->GetTags()) {
        m_TagToNodeMap[tag].push_back(node);
    }
}

void CEditorNodeTree::RemoveFromCache(FEditorNode* node) {
    m_IdToNodeMap.erase(node->GetID());

    auto& nameVec = m_NameToNodeMap[node->GetName()];
    nameVec.erase(std::remove(nameVec.begin(), nameVec.end(), node), nameVec.end());
    if (nameVec.empty()) {
        m_NameToNodeMap.erase(node->GetName());
    }

    auto& typeVec = m_TypeToNodeMap[node->GetType()];
    typeVec.erase(std::remove(typeVec.begin(), typeVec.end(), node), typeVec.end());
    if (typeVec.empty()) {
        m_TypeToNodeMap.erase(node->GetType());
    }

    for (const auto& tag : node->GetTags()) {
        auto& tagVec = m_TagToNodeMap[tag];
        tagVec.erase(std::remove(tagVec.begin(), tagVec.end(), node), tagVec.end());
        if (tagVec.empty()) {
            m_TagToNodeMap.erase(tag);
        }
    }
}

CallbackHandle CEditorNodeTree::RegisterEventCallback(FNodeEventCallback callback) {
    CallbackHandle handle = m_NextHandle++;
    m_EventCallbacks.push_back({ handle, std::move(callback) });
    return handle;
}

void CEditorNodeTree::UnregisterEventCallback(CallbackHandle handle) {
    auto it =
        std::find_if(m_EventCallbacks.begin(), m_EventCallbacks.end(),
                     [handle](const FCallbackEntry& entry) { return entry.Handle == handle; });

    if (it != m_EventCallbacks.end()) {
        m_EventCallbacks.erase(it);
    }
}

void CEditorNodeTree::UnregisterAllCallbacks() { m_EventCallbacks.clear(); }

void CEditorNodeTree::NotifyEvent(const FNodeEvent& event) {
    for (size_t i = 0; i < m_EventCallbacks.size(); ++i) {
        if (m_EventCallbacks[i].Callback) {
            m_EventCallbacks[i].Callback(event);
        }
    }
}

void CEditorNodeTree::OnNodeCreated(FEditorNode* node) {
    NotifyEvent(FNodeEvent(ENodeEventType::Created, node));
}

void CEditorNodeTree::OnNodeDeleted(FEditorNode* node) {
    NotifyEvent(FNodeEvent(ENodeEventType::Deleted, node));
}

void CEditorNodeTree::OnNodeRenamed(FEditorNode* node, const std::string& oldName) {
    NotifyEvent(FNodeEvent(ENodeEventType::Renamed, node, oldName));
}

void CEditorNodeTree::OnNodeParentChanged(FEditorNode* node, FEditorNode* oldParent) {
    NotifyEvent(FNodeEvent(ENodeEventType::Moved, node, oldParent));
}

void CEditorNodeTree::OnNodeSelected(FEditorNode* node) {
    NotifyEvent(FNodeEvent(ENodeEventType::Selected, node));
}
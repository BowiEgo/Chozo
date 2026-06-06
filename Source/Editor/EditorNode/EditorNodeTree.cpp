#include "EditorNodeTree.hpp"

#include <algorithm>

static EditorNode* CreateDemoTree(EditorNodeTree* tree) {
    const std::vector<std::string> fruitNames = { "Apple",      "Banana",    "Cherry", "Kiwi",
                                                  "Mango",      "Orange",    "Pear",   "Pineapple",
                                                  "Strawberry", "Watermelon" };
    const int multiplier                      = 2;

    auto rootNodeMask   = GET_NODE_MASK("Node_Root");
    auto sphereNodeMask = GET_NODE_MASK("Node_Regular", "ProceduralMesh_Sphere");
    auto cubeNodeMask   = GET_NODE_MASK("Node_Regular", "ProceduralMesh_Cube");

    if (EditorNodeRegistry::Test(sphereNodeMask, { "Node_Regular", "ProceduralMesh_Sphere" })) {
        CZ_CORE_LOG(Info, "sphere node mask test success");
    }

    auto root = CZ_NEW(CZ::MEMORY_USAGE_UI, EditorNode, "Root", rootNodeMask);

    for (size_t i = 0; i < fruitNames.size() * multiplier; ++i) {
        size_t fruitIndex = i / multiplier;
        int instance      = i % multiplier;

        std::string nodeName = fruitNames[fruitIndex] + " " + std::to_string(instance);

        EditorNode* fruitNode = tree->CreateNode(nodeName, sphereNodeMask, root);

        int childCount = static_cast<int>(nodeName.length());

        for (int j = 0; j < childCount; ++j) {
            std::string childName = "Child " + std::to_string(j);
            EditorNode* childNode = tree->CreateNode(childName, cubeNodeMask, fruitNode);
        }
    }

    return root;
}

EditorNodeTree::~EditorNodeTree() {
    Delete(m_RootNode);
    m_IdToNodeMap.clear();
    m_NameToNodesMap.clear();
    m_TypeToNodesMap.clear();
    m_TagToNodesMap.clear();
}

void EditorNodeTree::Init() {
    EditorNodeRegistry::Get().Init();

    m_RootNode = CreateDemoTree(this);
    UpdateNodeCache(m_RootNode);
}

EditorNode* EditorNodeTree::CreateNode(const std::string& name, TypeMask typeMask,
                                       EditorNode* parent) {
    if (!parent) {
        parent = m_RootNode;
    }

    auto node = CZ_NEW(CZ::MEMORY_USAGE_UI, EditorNode, name, typeMask);
    parent->AddChild(node);

    UpdateNodeCache(node);
    OnNodeCreated(node);

    return node;
}

void EditorNodeTree::DeleteNode(EditorNode* node) {
    if (!node || node == m_RootNode) return;

    EditorNode* parent = node->GetParent();
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

    Delete(node);
    m_IsDirty = true;
}

void EditorNodeTree::RenameNode(EditorNode* node, const std::string& newName) {
    if (!node) return;

    std::string oldName = node->GetName();
    if (oldName == newName) return;

    node->SetName(newName);

    // Update Cache
    auto& vec = m_NameToNodesMap[oldName];
    vec.erase(std::remove(vec.begin(), vec.end(), node), vec.end());
    if (vec.empty()) {
        m_NameToNodesMap.erase(oldName);
    }
    m_NameToNodesMap[newName].push_back(node);

    OnNodeRenamed(node, oldName);
    m_IsDirty = true;
}

void EditorNodeTree::SetNodeParent(EditorNode* node, EditorNode* newParent) {
    if (!node || node == m_RootNode) return;
    if (newParent == node->GetParent()) return;

    EditorNode* oldParent = node->GetParent();
    node->SetParent(newParent);

    OnNodeParentChanged(node, oldParent);
    m_IsDirty = true;
}

EditorNode* EditorNodeTree::QueryNodeByID(NodeID id) const {
    auto it = m_IdToNodeMap.find(id);
    return it != m_IdToNodeMap.end() ? it->second : nullptr;
}

EditorNode* EditorNodeTree::QueryNodeByName(const std::string& name) const {
    auto it = m_NameToNodesMap.find(name);
    return it != m_NameToNodesMap.end() && !it->second.empty() ? it->second[0] : nullptr;
}

std::vector<EditorNode*> EditorNodeTree::QueryNodesByType(Type type) const {
    std::vector<EditorNode*> result;
    auto it = m_TypeToNodesMap.find(type);
    return it != m_TypeToNodesMap.end() ? it->second : std::vector<EditorNode*>();
}

std::vector<EditorNode*> EditorNodeTree::QueryNodesByTypeMask(const TypeMask& queryMask) const {
    std::vector<EditorNode*> result;
    for (Type type : queryMask.GetSetIndices()) {
        auto it = m_TypeToNodesMap.find(type);
        if (it != m_TypeToNodesMap.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
    }
    return result;
}

std::vector<EditorNode*>
    EditorNodeTree::QueryNodesByTypeNames(std::initializer_list<std::string> names) const {
    std::vector<EditorNode*> result;
    for (auto name : names) {
        auto type = TypeRegister::Get().GetType(name);
        auto it   = m_TypeToNodesMap.find(type);
        if (it != m_TypeToNodesMap.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
    }
    return result;
}

std::vector<EditorNode*> EditorNodeTree::QueryNodesByTag(const std::string& tag) const {
    auto it = m_TagToNodesMap.find(tag);
    return it != m_TagToNodesMap.end() ? it->second : std::vector<EditorNode*>();
}

void EditorNodeTree::SelectNode(EditorNode* node, bool addToSelection) {
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

void EditorNodeTree::ClearSelection() {
    m_SelectedNodes.clear();
    m_SelectedNode = nullptr;
}

bool EditorNodeTree::IsSelected(EditorNode* node) const {
    return std::find(m_SelectedNodes.begin(), m_SelectedNodes.end(), node) != m_SelectedNodes.end();
}

void EditorNodeTree::ForEachNodeRecursive(std::function<void(EditorNode*)> callback) {
    std::function<void(EditorNode*)> traverse = [&](EditorNode* node) {
        callback(node);
        for (auto* child : node->GetChildren()) {
            traverse(child);
        }
    };
    traverse(m_RootNode);
}

void EditorNodeTree::UpdateNodeCache(EditorNode* node) {
    m_IdToNodeMap[node->GetID()] = node;
    m_NameToNodesMap[node->GetName()].push_back(node);

    for (Type type : node->GetTypeMask().GetSetIndices()) {
        m_TypeToNodesMap[type].push_back(node);
    }

    for (const auto& tag : node->GetTags()) {
        m_TagToNodesMap[tag].push_back(node);
    }
}

void EditorNodeTree::RemoveFromCache(EditorNode* node) {
    m_IdToNodeMap.erase(node->GetID());

    auto& nameVec = m_NameToNodesMap[node->GetName()];
    nameVec.erase(std::remove(nameVec.begin(), nameVec.end(), node), nameVec.end());
    if (nameVec.empty()) {
        m_NameToNodesMap.erase(node->GetName());
    }

    const TypeMask& mask      = node->GetTypeMask();
    std::vector<Type> indices = mask.GetSetIndices();
    for (Type type : indices) {
        auto it = m_TypeToNodesMap.find(type);
        if (it != m_TypeToNodesMap.end()) {
            auto& vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), node), vec.end());
            if (vec.empty()) {
                m_TypeToNodesMap.erase(it);
            }
        }
    }

    for (const auto& tag : node->GetTags()) {
        auto& tagVec = m_TagToNodesMap[tag];
        tagVec.erase(std::remove(tagVec.begin(), tagVec.end(), node), tagVec.end());
        if (tagVec.empty()) {
            m_TagToNodesMap.erase(tag);
        }
    }
}

CallbackHandle EditorNodeTree::RegisterEventCallback(NodeEventCallback callback) {
    CallbackHandle handle = m_NextHandle++;
    m_EventCallbacks.push_back({ handle, std::move(callback) });
    return handle;
}

void EditorNodeTree::UnregisterEventCallback(CallbackHandle handle) {
    auto it = std::find_if(m_EventCallbacks.begin(), m_EventCallbacks.end(),
                           [handle](const CallbackEntry& entry) { return entry.Handle == handle; });

    if (it != m_EventCallbacks.end()) {
        m_EventCallbacks.erase(it);
    }
}

void EditorNodeTree::UnregisterAllCallbacks() { m_EventCallbacks.clear(); }

void EditorNodeTree::NotifyEvent(const NodeEvent& event) {
    for (size_t i = 0; i < m_EventCallbacks.size(); ++i) {
        if (m_EventCallbacks[i].Callback) {
            m_EventCallbacks[i].Callback(event);
        }
    }
}

void EditorNodeTree::OnNodeCreated(EditorNode* node) {
    NotifyEvent(NodeEvent(EditorNodeEventType::Created, node));
}

void EditorNodeTree::OnNodeDeleted(EditorNode* node) {
    NotifyEvent(NodeEvent(EditorNodeEventType::Deleted, node));
}

void EditorNodeTree::OnNodeRenamed(EditorNode* node, const std::string& oldName) {
    NotifyEvent(NodeEvent(EditorNodeEventType::Renamed, node, oldName));
}

void EditorNodeTree::OnNodeParentChanged(EditorNode* node, EditorNode* oldParent) {
    NotifyEvent(NodeEvent(EditorNodeEventType::Moved, node, oldParent));
}

void EditorNodeTree::OnNodeSelected(EditorNode* node) {
    NotifyEvent(NodeEvent(EditorNodeEventType::Selected, node));
}
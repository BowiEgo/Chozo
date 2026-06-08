#include "SyncBridge.hpp"

#include <Runtime/RenderCore/Components/Components.hpp>

SyncBridge::SyncBridge(Scene scene) : m_Scene(scene) {
    CZ_CORE_ASSERT(m_Scene, "Scene cannot be null");
}

// ===== Node Registration =====
void SyncBridge::RegisterNode(EditorNode* node) {
    if (!node) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check if already registered
    if (m_NodeToEntity.find(node) != m_NodeToEntity.end()) {
        CZ_EDITOR_LOG(Warning, "Node already registered: {}", node->GetName());
        return;
    }

    // Create Entity in Scene
    Entity entity = m_Scene->CreateEntity(node->GetName());

    // Add Mesh component if applicable
    // if (node->HasMesh()) {
    // SyncMeshComponent(entity, node->GetMesh());
    // }

    // Add Material component if applicable
    // if (node->HasMaterial()) {
    //     SyncMaterialComponent(entity, node->GetMaterial());
    // }

    // Add Relationship component if parent exists
    EditorNode* parentNode = node->GetParent();
    if (parentNode) {
        auto parentIt = m_NodeToEntity.find(parentNode);
        if (parentIt != m_NodeToEntity.end()) {
            SyncRelationshipComponent(entity, parentIt->second);
        }
    }

    // Register mapping
    m_NodeToEntity[node]   = entity;
    m_EntityToNode[entity] = node;

    // Mark as dirty
    m_DirtyNodes.insert(node);

    // CZ_EDITOR_LOG(Trace, "Registered node '{}' -> Entity[{}]", node->GetName(),
    //               static_cast<uint32_t>(entity.GetHandle()));

    // Trigger callback
    if (m_OnNodeSynced) {
        m_OnNodeSynced(node, entity);
    }
}

void SyncBridge::UnregisterNode(EditorNode* node) {
    if (!node) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_NodeToEntity.find(node);
    if (it == m_NodeToEntity.end()) {
        CZ_EDITOR_LOG(Warning, "Node not registered: {}", node->GetName());
        return;
    }

    Entity entity = it->second;

    // Destroy Entity in Scene
    m_Scene->DestroyEntity(entity);

    // Remove mappings
    m_NodeToEntity.erase(it);
    m_EntityToNode.erase(entity);
    m_DirtyNodes.erase(node);
    m_DirtyEntities.erase(entity);

    CZ_EDITOR_LOG(Trace, "Unregistered node '{}'", node->GetName());
}

// ===== Synchronization =====
void SyncBridge::SyncAll() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Sync dirty nodes to entities
    for (auto* node : m_DirtyNodes) {
        SyncNodeToEntity(node);
    }

    // Sync dirty entities to nodes
    for (auto entity : m_DirtyEntities) {
        SyncEntityToNode(entity);
    }

    ClearDirty();
}

void SyncBridge::SyncNodeToEntity(EditorNode* node) {
    if (!node || node->IsRoot()) return;

    auto it = m_NodeToEntity.find(node);
    if (it == m_NodeToEntity.end()) {
        CZ_EDITOR_LOG(Warning, "Node not found: {}", node->GetName());
        return;
    }

    Entity entity = it->second;

    // // Sync Name
    // SyncName(node, entity);

    // Sync Components
    SyncComponents(node, entity);

    // Trigger callback
    if (m_OnNodeSynced) {
        m_OnNodeSynced(node, entity);
    }

    node->ClearDirty();
}

void SyncBridge::SyncEntityToNode(Entity entity) {
    if (!entity.IsValid()) return;

    auto it = m_EntityToNode.find(entity);
    if (it == m_EntityToNode.end()) {
        CZ_EDITOR_LOG(Warning, "Entity not found: {}", static_cast<uint32_t>(entity.GetHandle()));
        return;
    }

    EditorNode* node = it->second;

    // Sync Transform
    SyncTransform(entity, node);

    // Sync Name
    SyncName(entity, node);

    // Trigger callback
    if (m_OnEntitySynced) {
        m_OnEntitySynced(entity, node);
    }
}

// ===== Dirty Tracking =====
void SyncBridge::MarkDirty(EditorNode* node) {
    if (node) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DirtyNodes.insert(node);
    }
}

void SyncBridge::MarkDirty(Entity entity) {
    if (entity.IsValid()) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DirtyEntities.insert(entity);
    }
}

void SyncBridge::ClearDirty() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_DirtyNodes.clear();
    m_DirtyEntities.clear();
}

// ===== Mapping Queries =====
Entity SyncBridge::GetEntityFromNode(EditorNode* node) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_NodeToEntity.find(node);
    if (it != m_NodeToEntity.end()) {
        return it->second;
    }
    return Entity();
}

EditorNode* SyncBridge::GetNodeFromEntity(Entity entity) const {
    if (!entity.IsValid()) return nullptr;

    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_EntityToNode.find(entity);
    if (it != m_EntityToNode.end()) {
        return it->second;
    }
    return nullptr;
}

bool SyncBridge::HasMapping(EditorNode* node) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_NodeToEntity.find(node) != m_NodeToEntity.end();
}

bool SyncBridge::HasMapping(Entity entity) const {
    if (!entity.IsValid()) return false;

    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_EntityToNode.find(entity) != m_EntityToNode.end();
}

// ===== Batch Operations =====
void SyncBridge::SyncAllNodesToEntities() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& [node, entity] : m_NodeToEntity) {
        SyncNodeToEntity(node);
    }
}

void SyncBridge::SyncAllEntitiesToNodes() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& [entity, node] : m_EntityToNode) {
        SyncEntityToNode(entity);
    }
}

// ===== Internal Sync Helpers =====
void SyncBridge::SyncTransform(EditorNode* node, Entity entity) {}

void SyncBridge::SyncTransform(Entity entity, EditorNode* node) {}

void SyncBridge::SyncName(EditorNode* node, Entity entity) {
    if (!m_Scene->HasComponent<NameComponent>(entity)) {
        m_Scene->AddComponent<NameComponent>(entity, node->GetName());
    } else {
        auto& nameComp = m_Scene->GetComponent<NameComponent>(entity);
        if (nameComp.Name != node->GetName()) {
            nameComp.Name = node->GetName();
        }
    }
}

void SyncBridge::SyncName(Entity entity, EditorNode* node) {
    if (m_Scene->HasComponent<NameComponent>(entity)) {
        const auto& nameComp = m_Scene->GetComponent<NameComponent>(entity);
        if (node->GetName() != nameComp.Name) {
            node->SetName(nameComp.Name);
        }
    }
}

void SyncBridge::SyncComponents(EditorNode* node, Entity entity) {
    if (!node->IsDirty()) return;

    if (node->HasTransform()) {
        SyncTransformComponent(entity, node->GetTransformParams());
    } else {
        m_Scene->RemoveComponent<TransformComponent>(entity);
    }

    // Handle Mesh Component - store only parameters
    // if (node->HasMesh()) {
    //     SyncMeshComponent(entity, node->GetMeshParams());
    // } else if (m_Scene->HasComponent<MeshComponent>(entity)) {
    //     m_Scene->RemoveComponent<MeshComponent>(entity);
    // }
}

// ===== Component Creation Helpers =====
void SyncBridge::SyncTransformComponent(Entity entity, const TransformParams transformParams) {
    m_Scene->SetTransform(entity, transformParams);
}

void SyncBridge::SyncMeshComponent(Entity entity, const MeshParams params) {
    m_Scene->SetMesh(entity, params);
}

void SyncBridge::SyncRelationshipComponent(Entity entity, Entity parent) {
    m_Scene->SetParent(entity, parent);
}

// ===== Validation =====
bool SyncBridge::ValidateMapping(EditorNode* node) const {
    auto it = m_NodeToEntity.find(node);
    if (it == m_NodeToEntity.end()) return false;

    Entity entity = it->second;
    auto revIt    = m_EntityToNode.find(entity);
    return revIt != m_EntityToNode.end() && revIt->second == node;
}

bool SyncBridge::ValidateMapping(Entity entity) const {
    auto it = m_EntityToNode.find(entity);
    if (it == m_EntityToNode.end()) return false;

    EditorNode* node = it->second;
    auto revIt       = m_NodeToEntity.find(node);
    return revIt != m_NodeToEntity.end() && revIt->second == entity;
}

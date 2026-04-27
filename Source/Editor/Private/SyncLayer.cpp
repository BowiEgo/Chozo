#include "SyncLayer.h"
#include "Components.h"

FSyncLayer::FSyncLayer(FScene* scene) : m_Scene(scene) {
    CZ_CORE_ASSERT(m_Scene != nullptr, "Scene cannot be null");
}

// ===== Node Registration =====
void FSyncLayer::RegisterNode(FEditorNode* node) {
    if (!node) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check if already registered
    if (m_NodeToEntity.find(node) != m_NodeToEntity.end()) {
        CZ_LOG(LogSyncLayer, Warning, "Node already registered: {}", node->GetName());
        return;
    }

    // Create Entity in Scene
    FEntity entity = m_Scene->CreateEntity(node->GetName());

    // Add Mesh component if applicable
    // if (node->HasMesh()) {
    // SyncMeshComponent(entity, node->GetMesh());
    // }

    // Add Material component if applicable
    // if (node->HasMaterial()) {
    //     SyncMaterialComponent(entity, node->GetMaterial());
    // }

    // Add Relationship component if parent exists
    FEditorNode* parentNode = node->GetParent();
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

    CZ_LOG(LogSyncLayer, Trace, "Registered node '{}' -> Entity[{}]", node->GetName(),
           static_cast<uint32_t>(entity.GetHandle()));

    // Trigger callback
    if (m_OnNodeSynced) {
        m_OnNodeSynced(node, entity);
    }
}

void FSyncLayer::UnregisterNode(FEditorNode* node) {
    if (!node) return;

    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_NodeToEntity.find(node);
    if (it == m_NodeToEntity.end()) {
        CZ_LOG(LogSyncLayer, Warning, "Node not registered: {}", node->GetName());
        return;
    }

    FEntity entity = it->second;

    // Destroy Entity in Scene
    m_Scene->DestroyEntity(entity);

    // Remove mappings
    m_NodeToEntity.erase(it);
    m_EntityToNode.erase(entity);
    m_DirtyNodes.erase(node);
    m_DirtyEntities.erase(entity);

    CZ_LOG(LogSyncLayer, Trace, "Unregistered node '{}'", node->GetName());
}

// ===== Synchronization =====
void FSyncLayer::SyncAll() {
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

void FSyncLayer::SyncNodeToEntity(FEditorNode* node) {
    if (!node || node->IsRoot()) return;

    auto it = m_NodeToEntity.find(node);
    if (it == m_NodeToEntity.end()) {
        CZ_LOG(LogSyncLayer, Warning, "Node not found: {}", node->GetName());
        return;
    }

    FEntity entity = it->second;

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

void FSyncLayer::SyncEntityToNode(FEntity entity) {
    if (!entity.IsValid()) return;

    auto it = m_EntityToNode.find(entity);
    if (it == m_EntityToNode.end()) {
        CZ_LOG(LogSyncLayer, Warning, "Entity not found: {}",
               static_cast<uint32_t>(entity.GetHandle()));
        return;
    }

    FEditorNode* node = it->second;

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
void FSyncLayer::MarkDirty(FEditorNode* node) {
    if (node) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DirtyNodes.insert(node);
    }
}

void FSyncLayer::MarkDirty(FEntity entity) {
    if (entity.IsValid()) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DirtyEntities.insert(entity);
    }
}

void FSyncLayer::ClearDirty() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_DirtyNodes.clear();
    m_DirtyEntities.clear();
}

// ===== Mapping Queries =====
FEntity FSyncLayer::GetEntityFromNode(FEditorNode* node) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_NodeToEntity.find(node);
    if (it != m_NodeToEntity.end()) {
        return it->second;
    }
    return FEntity();
}

FEditorNode* FSyncLayer::GetNodeFromEntity(FEntity entity) const {
    if (!entity.IsValid()) return nullptr;

    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_EntityToNode.find(entity);
    if (it != m_EntityToNode.end()) {
        return it->second;
    }
    return nullptr;
}

bool FSyncLayer::HasMapping(FEditorNode* node) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_NodeToEntity.find(node) != m_NodeToEntity.end();
}

bool FSyncLayer::HasMapping(FEntity entity) const {
    if (!entity.IsValid()) return false;

    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_EntityToNode.find(entity) != m_EntityToNode.end();
}

// ===== Batch Operations =====
void FSyncLayer::SyncAllNodesToEntities() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& [node, entity] : m_NodeToEntity) {
        SyncNodeToEntity(node);
    }
}

void FSyncLayer::SyncAllEntitiesToNodes() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    for (auto& [entity, node] : m_EntityToNode) {
        SyncEntityToNode(entity);
    }
}

// ===== Internal Sync Helpers =====
void FSyncLayer::SyncTransform(FEditorNode* node, FEntity entity) {}

void FSyncLayer::SyncTransform(FEntity entity, FEditorNode* node) {}

void FSyncLayer::SyncName(FEditorNode* node, FEntity entity) {
    if (!m_Scene->HasComponent<FNameComponent>(entity)) {
        m_Scene->AddComponent<FNameComponent>(entity, node->GetName());
    } else {
        auto& nameComp = m_Scene->GetComponent<FNameComponent>(entity);
        if (nameComp.Name != node->GetName()) {
            nameComp.Name = node->GetName();
        }
    }
}

void FSyncLayer::SyncName(FEntity entity, FEditorNode* node) {
    if (m_Scene->HasComponent<FNameComponent>(entity)) {
        const auto& nameComp = m_Scene->GetComponent<FNameComponent>(entity);
        if (node->GetName() != nameComp.Name) {
            node->SetName(nameComp.Name);
        }
    }
}

void FSyncLayer::SyncComponents(FEditorNode* node, FEntity entity) {
    if (!node->IsDirty()) return;

    if (node->HasTransform()) {
        SyncTransformComponent(entity, node->GetTransformParams());
    } else {
        m_Scene->RemoveComponent<FTransformComponent>(entity);
    }

    // Handle Mesh Component - store only parameters
    if (node->HasMesh()) {
        SyncMeshComponent(entity, node->GetMeshProps());
    } else if (m_Scene->HasComponent<FMeshComponent>(entity)) {
        m_Scene->RemoveComponent<FMeshComponent>(entity);
    }
}

// ===== Component Creation Helpers =====
void FSyncLayer::SyncTransformComponent(FEntity entity, const FTransformParams* transformParams) {
    m_Scene->SetTransform(entity, *transformParams);
}

void FSyncLayer::SyncMeshComponent(FEntity entity, const FMeshProps* props) {
    m_Scene->SetMesh(entity, *props);
}

void FSyncLayer::SyncRelationshipComponent(FEntity entity, FEntity parent) {
    m_Scene->SetParent(entity, parent);
}

// ===== Validation =====
bool FSyncLayer::ValidateMapping(FEditorNode* node) const {
    auto it = m_NodeToEntity.find(node);
    if (it == m_NodeToEntity.end()) return false;

    FEntity entity = it->second;
    auto revIt     = m_EntityToNode.find(entity);
    return revIt != m_EntityToNode.end() && revIt->second == node;
}

bool FSyncLayer::ValidateMapping(FEntity entity) const {
    auto it = m_EntityToNode.find(entity);
    if (it == m_EntityToNode.end()) return false;

    FEditorNode* node = it->second;
    auto revIt        = m_NodeToEntity.find(node);
    return revIt != m_NodeToEntity.end() && revIt->second == entity;
}

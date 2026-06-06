#pragma once

#include <Runtime/RenderCore/Scene/Scene.hpp>

#include "EditorNode/EditorNode.hpp"

/**
 * Synchronization bridge between Editor Node tree and Runtime ECS
 *
 * Responsibilities:
 * - Maintain bidirectional mapping between Node and Entity
 * - Sync Transform changes
 * - Sync Component additions/removals
 * - Handle undo/redo operations via command pattern
 * - Manage dirty tracking for efficient updates
 */
class SyncBridge {
public:
    explicit SyncBridge(Scene scene);
    ~SyncBridge() = default;

    // ===== Node Registration =====
    // Register a new node and create corresponding Entity
    void RegisterNode(EditorNode* node);

    // Unregister a node and destroy its Entity
    void UnregisterNode(EditorNode* node);

    // ===== Synchronization =====
    // Sync all dirty nodes/entities
    void SyncAll();

    // Sync specific node to entity
    void SyncNodeToEntity(EditorNode* node);

    // Sync specific entity to node
    void SyncEntityToNode(Entity entity);

    // ===== Dirty Tracking =====
    void MarkDirty(EditorNode* node);
    void MarkDirty(Entity entity);
    void ClearDirty();

    // ===== Mapping Queries =====
    Entity GetEntityFromNode(EditorNode* node) const;
    EditorNode* GetNodeFromEntity(Entity entity) const;
    bool HasMapping(EditorNode* node) const;
    bool HasMapping(Entity entity) const;

    // ===== Batch Operations =====
    // Sync all nodes to entities (useful after loading a scene)
    void SyncAllNodesToEntities();

    // Sync all entities to nodes (useful after physics simulation)
    void SyncAllEntitiesToNodes();

    // ===== Callbacks =====
    using OnNodeSyncedCallback = std::function<void(EditorNode*, Entity)>;
    void SetOnNodeSynced(OnNodeSyncedCallback callback) { m_OnNodeSynced = callback; }

    using OnEntitySyncedCallback = std::function<void(Entity, EditorNode*)>;
    void SetOnEntitySynced(OnEntitySyncedCallback callback) { m_OnEntitySynced = callback; }

private:
    // ===== Internal Sync Helpers =====
    void SyncTransform(EditorNode* node, Entity entity);
    void SyncTransform(Entity entity, EditorNode* node);

    void SyncName(EditorNode* node, Entity entity);
    void SyncName(Entity entity, EditorNode* node);

    void SyncComponents(EditorNode* node, Entity entity);

    // ===== Component Creation Helpers =====
    void SyncTransformComponent(Entity entity, const TransformParams transformParams);
    void SyncMeshComponent(Entity entity, const MeshParams props);
    // void SyncMaterialComponent(Entity entity, const FMaterialComponent& material);
    void SyncRelationshipComponent(Entity entity, Entity parent);

    // ===== Validation =====
    bool ValidateMapping(EditorNode* node) const;
    bool ValidateMapping(Entity entity) const;

private:
    Scene m_Scene;

    // Bidirectional mappings
    std::unordered_map<EditorNode*, Entity> m_NodeToEntity;
    std::unordered_map<Entity, EditorNode*> m_EntityToNode;

    // Dirty tracking
    std::unordered_set<EditorNode*> m_DirtyNodes;
    std::unordered_set<Entity> m_DirtyEntities;

    // Thread safety (if used from multiple threads)
    mutable std::mutex m_Mutex;

    // Callbacks
    OnNodeSyncedCallback m_OnNodeSynced;
    OnEntitySyncedCallback m_OnEntitySynced;
};
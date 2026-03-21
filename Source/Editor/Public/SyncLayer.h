#pragma once

#include "EditorNode.h"
#include "Scene.h"

/**
 * Synchronization layer between Editor Node tree and Runtime ECS
 *
 * Responsibilities:
 * - Maintain bidirectional mapping between Node and Entity
 * - Sync Transform changes
 * - Sync Component additions/removals
 * - Handle undo/redo operations via command pattern
 * - Manage dirty tracking for efficient updates
 */
class EDITOR_API FSyncLayer {
public:
    explicit FSyncLayer(FScene* scene);
    ~FSyncLayer() = default;

    // ===== Node Registration =====
    // Register a new node and create corresponding Entity
    void RegisterNode(FEditorNode* node);

    // Unregister a node and destroy its Entity
    void UnregisterNode(FEditorNode* node);

    // ===== Synchronization =====
    // Sync all dirty nodes/entities
    void SyncAll();

    // Sync specific node to entity
    void SyncNodeToEntity(FEditorNode* node);

    // Sync specific entity to node
    void SyncEntityToNode(FEntity entity);

    // ===== Dirty Tracking =====
    void MarkDirty(FEditorNode* node);
    void MarkDirty(FEntity entity);
    void ClearDirty();

    // ===== Mapping Queries =====
    FEntity GetEntityFromNode(FEditorNode* node) const;
    FEditorNode* GetNodeFromEntity(FEntity entity) const;
    bool HasMapping(FEditorNode* node) const;
    bool HasMapping(FEntity entity) const;

    // ===== Batch Operations =====
    // Sync all nodes to entities (useful after loading a scene)
    void SyncAllNodesToEntities();

    // Sync all entities to nodes (useful after physics simulation)
    void SyncAllEntitiesToNodes();

    // ===== Callbacks =====
    using OnNodeSyncedCallback = std::function<void(FEditorNode*, FEntity)>;
    void SetOnNodeSynced(OnNodeSyncedCallback callback) { m_OnNodeSynced = callback; }

    using OnEntitySyncedCallback = std::function<void(FEntity, FEditorNode*)>;
    void SetOnEntitySynced(OnEntitySyncedCallback callback) { m_OnEntitySynced = callback; }

private:
    // ===== Internal Sync Helpers =====
    void SyncTransform(FEditorNode* node, FEntity entity);
    void SyncTransform(FEntity entity, FEditorNode* node);

    void SyncName(FEditorNode* node, FEntity entity);
    void SyncName(FEntity entity, FEditorNode* node);

    void SyncComponents(FEditorNode* node, FEntity entity);

    // ===== Component Creation Helpers =====
    void AddTransformComponent(FEntity entity, const FTransformComponent& transform);
    // void AddMeshComponent(FEntity entity, const FMeshComponent& mesh);
    // void AddMaterialComponent(FEntity entity, const FMaterialComponent& material);
    void AddRelationshipComponent(FEntity entity, FEntity parent);

    // ===== Validation =====
    bool ValidateMapping(FEditorNode* node) const;
    bool ValidateMapping(FEntity entity) const;

private:
    FScene* m_Scene = nullptr;

    // Bidirectional mappings
    std::unordered_map<FEditorNode*, FEntity> m_NodeToEntity;
    std::unordered_map<FEntity, FEditorNode*> m_EntityToNode;

    // Dirty tracking
    std::unordered_set<FEditorNode*> m_DirtyNodes;
    std::unordered_set<FEntity> m_DirtyEntities;

    // Thread safety (if used from multiple threads)
    mutable std::mutex m_Mutex;

    // Callbacks
    OnNodeSyncedCallback m_OnNodeSynced;
    OnEntitySyncedCallback m_OnEntitySynced;
};
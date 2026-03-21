# Chozo Engine Scene Architecture Design Document

## 1. Overview

### 1.1 Design Goals

- **High Performance**: Based on ECS architecture, maximize cache hit rate, support massive numbers of entities
- **Intuitiveness**: Retain scene tree abstraction, provide friendly editor experience
- **Flexibility**: Support both static scene organization and dynamic runtime parent-child relationships
- **Extensibility**: Easy to add new components and systems

### 1.2 Core Concepts

| Concept                    | Responsibility                          | Lifecycle        | Serializable |
| -------------------------- | --------------------------------------- | ---------------- | ------------ |
| **Scene**                  | Container managing Entities and Systems | Runtime + Disk   | ✅           |
| **Entity**                 | Handle, unique identifier               | Runtime          | ❌ (ID only) |
| **Component**              | Pure data                               | Runtime          | ✅           |
| **System**                 | Logic processing                        | Runtime          | ❌           |
| **Node**                   | Editor abstraction, organizes hierarchy | Editor + Runtime | ✅           |
| **Relationship Component** | Runtime parent-child relationship       | Runtime          | ✅           |

### 1.3 Architecture Diagram

```text
┌─────────────────────────────────────────────────────────────────────┐
│                           Asset System                              │
├─────────────────────────────────────────────────────────────────────┤
│  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐            │
│  │  Mesh Asset   │  │ Texture Asset │  │ Material Asset│            │
│  └───────────────┘  └───────────────┘  └───────────────┘            │
│                              │                                      │
│                              ▼                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                        Scene Asset                            │  │
│  │  ┌─────────────────────────────────────────────────────────┐  │  │
│  │  │                    Registry (entt)                      │  │  │
│  │  │  ┌────────────┐  ┌────────────┐  ┌────────────┐         │  │  │
│  │  │  │ Transform  │  │  Velocity  │  │   Render   │         │  │  │
│  │  │  │ Component  │  │ Component  │  │ Component  │         │  │  │
│  │  │  └────────────┘  └────────────┘  └────────────┘         │  │  │
│  │  └─────────────────────────────────────────────────────────┘  │  │
│  │                                                               │  │
│  │  ┌─────────────────────────────────────────────────────────┐  │  │
│  │  │                     System Queue                        │  │  │
│  │  │  [Movement] [Render] [Physics] [Audio] [AI]             │  │  │
│  │  └─────────────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘

                                    │
                                    ▼

┌─────────────────────────────────────────────────────────────────────┐
│                          Editor Layer                               │
├─────────────────────────────────────────────────────────────────────┤
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                         Scene Tree                            │  │
│  │  ┌─────────────────────────────────────────────────────────┐  │  │
│  │  │                        FNode                            │  │  │
│  │  │  ┌─────────────────────────────────────────────────┐    │  │  │
│  │  │  │  Name: "World"                                  │    │  │  │
│  │  │  │  Transform                                      │    │  │  │
│  │  │  │  Children: [Node1, Node2, Node3]                │    │  │  │
│  │  │  │  Entity: <reference to runtime Entity>          │    │  │  │
│  │  │  └─────────────────────────────────────────────────┘    │  │  │
│  │  └─────────────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                        Sync Layer                             │  │
│  │                    Node  ◄──────►  Entity                     │  │
│  │              (Bidirectional Synchronization)                  │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Core Type Definitions

### 2.1 Entity (Pure Handle)

```cpp
// Entity.h
#pragma once
#include <entt/entt.hpp>

class FEntity {
public:
    FEntity() = default;
    explicit FEntity(entt::entity handle) : m_Handle(handle) {}

    entt::entity GetHandle() const { return m_Handle; }
    bool IsValid() const { return m_Handle != entt::null; }

    bool operator==(const FEntity& other) const {
        return m_Handle == other.m_Handle;
    }

private:
    entt::entity m_Handle = entt::null;
};

// Hash support
namespace std {
    template<> struct hash<FEntity> {
        size_t operator()(const FEntity& entity) const {
            return hash<entt::entity>()(entity.GetHandle());
        }
    };
}
```

### 2.2 Component (Data Components)

```cpp
// TransformComponent.h
#pragma once
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix4.h"
#include "Math/MathUtils.h"

struct FTransformComponent {
    FVector3 Translation = FVector3::Zero;
    FQuaternion Rotation = FQuaternion::Identity();
    FVector3 Scale = FVector3::One;

    // Optional dirty flag for ECS synchronization optimization
    mutable uint32_t Revision = 0;

    FTransformComponent() = default;
    explicit FTransformComponent(const FVector3& translation) : Translation(translation) {}

    FMatrix4 GetMatrix() const {
        return FMatrix4::Translate(Translation) *
               Rotation.ToMatrix() *
               FMatrix4::Scale(Scale);
    }

    void SetMatrix(const FMatrix4& matrix) {
        Math::DecomposeTransform(matrix, Translation, Rotation, Scale);
        Revision++;
    }

    void SetTranslation(const FVector3& translation) {
        if (Translation != translation) {
            Translation = translation;
            Revision++;
        }
    }

    FVector3 GetForward() const { return Rotation * FVector3::Forward; }
    FVector3 GetRight() const { return Rotation * FVector3::Right; }
    FVector3 GetUp() const { return Rotation * FVector3::Up; }
};
```

```cpp
// VelocityComponent.h
#pragma once
#include "Math/Vector3.h"

struct FVelocityComponent {
    FVector3 LinearVelocity = FVector3::Zero;
    FVector3 AngularVelocity = FVector3::Zero;

    float LinearDamping = 0.0f;
    float AngularDamping = 0.0f;

    mutable uint32_t Revision = 0;

    FVelocityComponent() = default;
    explicit FVelocityComponent(const FVector3& linear) : LinearVelocity(linear) {}

    void SetLinearVelocity(const FVector3& velocity) {
        if (LinearVelocity != velocity) {
            LinearVelocity = velocity;
            Revision++;
        }
    }
};
```

```cpp
// RelationshipComponent.h
#pragma once
#include "ECS/Entity.h"
#include <vector>

struct FRelationshipComponent {
    FEntity Parent;
    std::vector<FEntity> Children;

    bool HasParent() const { return Parent.IsValid(); }
    bool HasChildren() const { return !Children.empty(); }

    void AddChild(FEntity child) {
        Children.push_back(child);
    }

    void RemoveChild(FEntity child) {
        Children.erase(
            std::remove(Children.begin(), Children.end(), child),
            Children.end()
        );
    }
};
```

### 2.3 Scene (Container)

```cpp
// Scene.h
#pragma once
#include "Asset/Asset.h"
#include "ECS/Entity.h"
#include "entt/entt.hpp"

class FScene : public FAsset {
public:
    FScene() = default;
    ~FScene() = default;

    // ----- Entity Management -----
    FEntity CreateEntity(const std::string& name = "");
    void DestroyEntity(FEntity entity);
    bool IsValid(FEntity entity) const;

    // ----- Component Operations -----
    template<typename T, typename... Args>
    T& AddComponent(FEntity entity, Args&&... args) {
        CZ_CORE_ASSERT(!HasComponent<T>(entity), "Component already exists");
        return m_Registry.emplace<T>(entity.GetHandle(), std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(FEntity entity) {
        CZ_CORE_ASSERT(HasComponent<T>(entity), "Component does not exist");
        m_Registry.erase<T>(entity.GetHandle());
    }

    template<typename T>
    T& GetComponent(FEntity entity) {
        CZ_CORE_ASSERT(HasComponent<T>(entity), "Component does not exist");
        return m_Registry.get<T>(entity.GetHandle());
    }

    template<typename T>
    bool HasComponent(FEntity entity) const {
        return m_Registry.all_of<T>(entity.GetHandle());
    }

    // ----- Query System -----
    template<typename... Components>
    auto View() {
        return m_Registry.view<Components...>();
    }

    template<typename... Components>
    auto View() const {
        return m_Registry.view<Components...>();
    }

    // ----- Relationship -----
    FEntity GetParent(FEntity entity);
    void SetParent(FEntity child, FEntity parent);
    std::vector<FEntity> GetChildren(FEntity entity);

    // ----- Serialization -----
    void Serialize(FArchive& ar) override;
    void Deserialize(FArchive& ar) override;

private:
    entt::registry m_Registry;
    std::string m_Name;
    UUID m_ID;
};
```

### 2.4 Node (Editor Node)

```cpp
// EditorNode.h
#pragma once
#include "ECS/Entity.h"
#include "Math/Matrix4.h"
#include <vector>
#include <string>

class FEditorNode {
public:
    FEditorNode() = default;
    explicit FEditorNode(const std::string& name) : m_Name(name) {}

    // ----- Hierarchy -----
    void AddChild(FEditorNode* child);
    void RemoveChild(FEditorNode* child);
    FEditorNode* GetParent() const { return m_Parent; }
    const std::vector<FEditorNode*>& GetChildren() const { return m_Children; }

    // ----- Transform -----
    FTransformComponent& GetTransform() { return m_Transform; }
    const FTransformComponent& GetTransform() const { return m_Transform; }

    // ----- Runtime Link -----
    void SetEntity(FEntity entity) { m_Entity = entity; }
    FEntity GetEntity() const { return m_Entity; }

    // ----- Properties -----
    void SetName(const std::string& name) { m_Name = name; }
    const std::string& GetName() const { return m_Name; }

private:
    std::string m_Name;
    FTransformComponent m_Transform;
    FEditorNode* m_Parent = nullptr;
    std::vector<FEditorNode*> m_Children;
    FEntity m_Entity;
};
```

## 3. System

### 3.1 Base Class Definition

```cpp
// System.h
#pragma once
#include "Scene/Scene.h"

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void Update(FScene* scene, float deltaTime) = 0;
    virtual void OnSceneChanged(FScene* scene) {}
    virtual const char* GetName() const = 0;
};
```

### 3.2 MovementSystem Implementation

```cpp
// MovementSystem.h
#pragma once
#include "ECS/System.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/VelocityComponent.h"

class FMovementSystem : public ISystem {
public:
    void Update(FScene* scene, float deltaTime) override;
    const char* GetName() const override { return "MovementSystem"; }
};
```

```cpp
// MovementSystem.cpp
#include "MovementSystem.h"

void FMovementSystem::Update(FScene* scene, float deltaTime) {
    auto view = scene->View<FTransformComponent, FVelocityComponent>();

    for (auto entity : view) {
        auto& [transform, velocity] = view.get<FTransformComponent, FVelocityComponent>(entity);

        // Update position
        transform.Translation += velocity.LinearVelocity * deltaTime;

        // Update rotation
        if (velocity.AngularVelocity.LengthSquared() > 0.0f) {
            FVector3 rotationDelta = velocity.AngularVelocity * deltaTime;
            FQuaternion deltaRot = FQuaternion::FromEuler(rotationDelta);
            transform.Rotation = transform.Rotation * deltaRot;
            transform.Rotation.Normalize();
        }

        // Mark dirty
        transform.Revision++;
    }
}
```

## 4. Sync Layer

### 4.1 Design Goals

- Maintain consistency between editor Node tree and runtime ECS data

- Support undo/redo operations

- Provide smooth editor experience

### 4.2 Sync Layer Implementation

```cpp
// SyncLayer.h
#pragma once
#include "Scene/Scene.h"
#include "Editor/EditorNode.h"
#include <unordered_map>

class FSyncLayer {
public:
    FSyncLayer(FScene* scene) : m_Scene(scene) {}

    // ----- Sync Operations -----
    void SyncNodeToEntity(FEditorNode* node);
    void SyncEntityToNode(FEntity entity);
    void SyncAll();

    // ----- Mapping -----
    void RegisterMapping(FEditorNode* node, FEntity entity);
    FEntity GetEntityFromNode(FEditorNode* node) const;
    FEditorNode* GetNodeFromEntity(FEntity entity) const;

    // ----- Dirty Tracking -----
    void MarkDirty(FEditorNode* node);
    void MarkDirty(FEntity entity);

private:
    FScene* m_Scene;
    std::unordered_map<FEditorNode*, FEntity> m_NodeToEntity;
    std::unordered_map<FEntity, FEditorNode*> m_EntityToNode;
    std::unordered_set<FEditorNode*> m_DirtyNodes;
    std::unordered_set<FEntity> m_DirtyEntities;
};
```

### 4.3 Workflow

```text
┌─────────────────────────────────────────────────────────────────┐
│                       Editor Operation                          │
│  (User drags Node to new position)                              │
└─────────────────────────────┬───────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                     Create Command                              │
│  MoveNodeCommand(originalPosition, newPosition)                 │
└─────────────────────────────┬───────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                     Execute Command                             │
│  1. Update Node's Transform                                     │
│  2. MarkDirty(node)                                             │
└─────────────────────────────┬───────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                     Sync to Entity                              │
│  SyncLayer::SyncNodeToEntity(node)                              │
│  Updates corresponding Entity's TransformComponent              │
└─────────────────────────────┬───────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                     ECS Systems                                 │
│  MovementSystem, PhysicsSystem, etc process                     │
└─────────────────────────────┬───────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                     Render                                      │
└─────────────────────────────────────────────────────────────────┘
```

## 5. Serialization

### 5.1 Asset Base Class

```cpp
// Asset.h
#pragma once
#include "Core/UUID.h"

class FAsset {
public:
    virtual ~FAsset() = default;

    UUID GetID() const { return m_ID; }
    void SetID(const UUID& id) { m_ID = id; }

    virtual void Serialize(FArchive& ar) = 0;
    virtual void Deserialize(FArchive& ar) = 0;

protected:
    UUID m_ID;
};
```

### 5.2 Scene Serialization Format

```json
{
    "Version": 1,
    "Name": "MyScene",
    "Entities": [
        {
            "ID": "123e4567-e89b-12d3-a456-426614174000",
            "Components": {
                "TransformComponent": {
                    "Translation": [0.0, 0.0, 0.0],
                    "Rotation": [0.0, 0.0, 0.0, 1.0],
                    "Scale": [1.0, 1.0, 1.0]
                },
                "MeshComponent": {
                    "MeshAssetID": "mesh_123",
                    "MaterialAssetID": "material_456"
                }
            }
        }
    ],
    "NodeTree": {
        "Root": {
            "Name": "World",
            "Transform": {...},
            "EntityID": "123e4567-e89b-12d3-a456-426614174000",
            "Children": [...]
        }
    }
}
```

## 6. Usage Examples

### 6.1 Creating Scene and Entities

```cpp
// Create scene
FScene scene;

// Create entities
FEntity player = scene.CreateEntity("Player");
FEntity enemy = scene.CreateEntity("Enemy");

// Add components
scene.AddComponent<FTransformComponent>(player, FVector3(0, 0, 0));
scene.AddComponent<FVelocityComponent>(player, FVector3(5, 0, 0));
scene.AddComponent<FTransformComponent>(enemy, FVector3(10, 0, 0));

// Set parent-child relationship
scene.SetParent(enemy, player);
```

### 6.2 Running Systems

```cpp
// Create system
FMovementSystem movementSystem;

// Game loop
float deltaTime = 0.016f;
movementSystem.Update(&scene, deltaTime);
```

## 6.3 Editor Integration

```cpp
// Create editor node tree
FEditorNode* root = new FEditorNode("World");
FEditorNode* playerNode = new FEditorNode("Player");
root->AddChild(playerNode);

// Create sync layer
FSyncLayer syncLayer(&scene);
syncLayer.RegisterMapping(playerNode, player);

// Sync
syncLayer.SyncAll();
```

## 7. Best Practices

### 7.1 Performance Optimization

- Batch Operations: When iterating with View(), avoid modifying components inside the loop
- Component Size: Keep Components within 64 bytes for better cache hit rate
- Dirty Flags: Use Revision field to track changes and reduce unnecessary synchronization

### 7.2 Debugging Techniques

- Add FNameComponent to store human-readable names for Entities
- Use entt::debug to inspect component layouts
- Enable ECS assertion checks

### 7.3 Extension Recommendations

- Custom Systems: Inherit from ISystem to implement business logic
- Custom Components: Keep them pure data, avoid virtual functions
- Asset References: Use UUID to reference other Assets, not raw pointers

## 8. Summary

| Component       | Responsibility     | Key Characteristics                            |
| --------------- | ------------------ | ---------------------------------------------- |
| **FEntity**     | Handle             | 8 bytes, pure identifier                       |
| **Component**   | Data               | Pure data, serializable                        |
| **FScene**      | Container          | Manages Registry, provides operation interface |
| **FEditorNode** | Editor abstraction | Organizes scene tree, associates with Entity   |
| **FSyncLayer**  | Sync bridge        | Bidirectional sync Node ↔ Entity               |
| **ISystem**     | Logic processing   | Independent from data, extensible              |

### Key advantages of this architecture:

- ✅ Runtime high performance (ECS cache-friendly)
- ✅ Editor intuitiveness (scene tree organization)
- ✅ Serializability (Asset system)
- ✅ Extensibility (independent systems)
- ✅ Decoupling (Entity does not hold Scene pointer)

// Scene / ECS behaviour that does not need a GPU.
//
// `SceneObj` resolves mesh assets through the registry it was configured with, so these tests
// create one explicitly instead of relying on the application singleton.
#include <string>
#include <vector>

#include <Runtime/RenderCore/Components/Components.hpp>
#include <Runtime/RenderCore/MeshRegistry.hpp>
#include <Runtime/RenderCore/ProceduralMesh/CubeParamsObj.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>

#include <doctest/doctest.h>

using namespace CZ;

namespace {

/// Creates a scene with the mesh registry the mesh-related cases need.
struct TestScene {
    MeshRegistry Registry;
    Scope<SceneObj> SceneOwner;
    SceneObj* Scene = nullptr;

    TestScene() {
        Registry.Init();
        SceneOwner = Scene::Create();
        Scene      = SceneOwner.get();
        Scene->SetMeshRegistry(&Registry);
    }

    ~TestScene() { Registry.Shutdown(); }
};

Scope<CubeParamsObj> MakeCubeParams() {
    return CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, CubeParamsObj, 1.0f, 1.0f, 1.0f, 1, 1, 1);
}

} // namespace

TEST_SUITE("Scene") {

    TEST_CASE("A new entity gets transform, name and relationship components") {
        TestScene scene;

        const Entity entity = scene.Scene->CreateEntity("Hero");

        CHECK(scene.Scene->IsValid(entity));
        CHECK(scene.Scene->HasComponent<TransformComponent>(entity));
        CHECK(scene.Scene->HasComponent<NameComponent>(entity));
        CHECK(scene.Scene->HasComponent<RelationshipComponent>(entity));
        CHECK_EQ(scene.Scene->GetComponent<NameComponent>(entity).Name, std::string("Hero"));
    }

    TEST_CASE("An unnamed entity gets a generated name") {
        TestScene scene;

        const Entity entity = scene.Scene->CreateEntity();

        CHECK_FALSE(scene.Scene->GetComponent<NameComponent>(entity).Name.empty());
    }

    TEST_CASE("Destroying an entity invalidates it") {
        TestScene scene;

        const Entity entity = scene.Scene->CreateEntity("Gone");
        scene.Scene->DestroyEntity(entity);

        CHECK_FALSE(scene.Scene->IsValid(entity));
    }

    TEST_CASE("Reparenting moves the child between the relationship lists") {
        TestScene scene;

        const Entity child  = scene.Scene->CreateEntity("Child");
        const Entity first  = scene.Scene->CreateEntity("First");
        const Entity second = scene.Scene->CreateEntity("Second");

        scene.Scene->SetParent(child, first);
        CHECK_EQ(scene.Scene->GetParent(child), first);
        CHECK_EQ(scene.Scene->GetChildren(first).size(), 1u);

        scene.Scene->SetParent(child, second);
        CHECK_EQ(scene.Scene->GetParent(child), second);
        CHECK_EQ(scene.Scene->GetChildren(first).size(), 0u);
        REQUIRE_EQ(scene.Scene->GetChildren(second).size(), 1u);
        CHECK_EQ(scene.Scene->GetChildren(second).front(), child);
    }

    TEST_CASE("Reparenting to nothing keeps the entity intact") {
        TestScene scene;

        const Entity child = scene.Scene->CreateEntity("Child");
        const Entity other = scene.Scene->CreateEntity("Other");
        scene.Scene->SetParent(child, other);

        scene.Scene->SetParent(child, Entity());

        CHECK_FALSE(scene.Scene->GetParent(child).IsValid());
        CHECK_EQ(scene.Scene->GetChildren(other).size(), 0u);
        CHECK(scene.Scene->IsValid(child));
    }

    TEST_CASE("Destroying a parent detaches its children") {
        TestScene scene;

        const Entity parent = scene.Scene->CreateEntity("Parent");
        const Entity child  = scene.Scene->CreateEntity("Child");
        scene.Scene->SetParent(child, parent);

        scene.Scene->DestroyEntity(parent);

        CHECK(scene.Scene->IsValid(child));
        CHECK_FALSE(scene.Scene->GetParent(child).IsValid());
    }

    TEST_CASE("Destroying a child removes it from its parent's list") {
        TestScene scene;

        const Entity parent = scene.Scene->CreateEntity("Parent");
        const Entity child  = scene.Scene->CreateEntity("Child");
        scene.Scene->SetParent(child, parent);
        REQUIRE_EQ(scene.Scene->GetChildren(parent).size(), 1u);

        scene.Scene->DestroyEntity(child);

        CHECK(scene.Scene->IsValid(parent));
        CHECK_EQ(scene.Scene->GetChildren(parent).size(), 0u);
    }

    TEST_CASE("Mesh parameters are turned into a mesh asset") {
        TestScene scene;

        const Entity entity = scene.Scene->CreateEntity("Mesh");
        auto params         = MakeCubeParams();
        scene.Scene->SetMesh(entity, MeshParams(params.get()));

        auto& meshComp = scene.Scene->GetComponent<MeshComponent>(entity);
        REQUIRE(meshComp.IsValid());

        Mesh mesh = scene.Registry.GetAsset(meshComp.m_Handle);
        REQUIRE(!!mesh);
        // Comparing handles: an unset handle must not look equal to the resolved one.
        const MeshParams resolved = mesh->GetParams();
        CHECK(resolved);
        CHECK(!!(resolved != MeshParams()));
        CHECK(resolved->GetTypeName() == "Cube");

        // Scene::Update() would upload the mesh to the GPU, which needs a device; the CPU side
        // data is generated while creating the asset.
        CHECK_GT(mesh->MeshBuffer.GetVertexCount(), 0u);
    }

    TEST_CASE("Render data carries the mesh and its world transform") {
        TestScene scene;

        const Entity entity = scene.Scene->CreateEntity("Mesh");
        auto params         = MakeCubeParams();
        scene.Scene->SetMesh(entity, MeshParams(params.get()));

        auto& transform = scene.Scene->GetComponent<TransformComponent>(entity);
        transform.SetTranslation(Vector3(1.0f, 2.0f, 3.0f));
        transform.WorldMatrix = transform.GetLocalMatrix();

        const std::vector<RenderData> renderData = scene.Scene->GetRenderDatas();
        REQUIRE_EQ(renderData.size(), 1u);
        CHECK(!!renderData.front().Mesh);
        CHECK_EQ(renderData.front().PushConstants.ModelMatrix.GetTranslation(),
                 Vector3(1.0f, 2.0f, 3.0f));
    }
}

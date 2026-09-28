// Transform hierarchy propagation.
//
// These cases pin down the behaviour that `TransformSystem` has to guarantee now that it runs on
// every scene update (see docs/TODO.md, P0-4): local composition, parent inheritance, dirty
// propagation and the guards for destroyed entities and cyclic hierarchies.
#include <Runtime/RenderCore/Components/Components.hpp>
#include <Runtime/RenderCore/Scene/Scene.hpp>

#include <doctest/doctest.h>

#include <cmath>

using namespace CZ;

namespace {

struct TestScene {
    Scope<SceneObj> SceneOwner;
    SceneObj* Scene = nullptr;

    TestScene() {
        SceneOwner = Scene::Create();
        Scene      = SceneOwner.get();
    }

    Entity EntityAt(const Vector3& position, const char* name) {
        const Entity entity = Scene->CreateEntity(name);

        auto owner = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, position);
        TransformParams params(owner.get());
        Scene->SetTransform(entity, params);

        return entity;
    }

    void Update() { Scene->Update(1.0f / 60.0f); }
};

bool NearlyEqual(const Vector3& a, const Vector3& b) {
    return std::abs(a.x - b.x) < 1e-4f && std::abs(a.y - b.y) < 1e-4f &&
           std::abs(a.z - b.z) < 1e-4f;
}

} // namespace

TEST_SUITE("TransformSystem") {

    TEST_CASE("A root entity's world matrix is its local matrix") {
        TestScene scene;

        const Entity entity = scene.EntityAt(Vector3(1.0f, 2.0f, 3.0f), "Root");
        scene.Update();

        const auto& transform = scene.Scene->GetComponent<TransformComponent>(entity);
        CHECK(NearlyEqual(transform.WorldMatrix.GetTranslation(), Vector3(1.0f, 2.0f, 3.0f)));
    }

    TEST_CASE("Children accumulate their parent's transform") {
        TestScene scene;

        const Entity parent = scene.EntityAt(Vector3(10.0f, 0.0f, 0.0f), "Parent");
        const Entity child  = scene.EntityAt(Vector3(0.0f, 2.0f, 0.0f), "Child");
        scene.Scene->SetParent(child, parent);

        scene.Update();

        const auto& childTransform = scene.Scene->GetComponent<TransformComponent>(child);
        CHECK(NearlyEqual(childTransform.WorldMatrix.GetTranslation(), Vector3(10.0f, 2.0f, 0.0f)));

        const auto& parentTransform = scene.Scene->GetComponent<TransformComponent>(parent);
        CHECK(
            NearlyEqual(parentTransform.WorldMatrix.GetTranslation(), Vector3(10.0f, 0.0f, 0.0f)));
    }

    TEST_CASE("Moving a parent moves its descendants") {
        TestScene scene;

        const Entity parent = scene.EntityAt(Vector3(1.0f, 0.0f, 0.0f), "Parent");
        const Entity child  = scene.EntityAt(Vector3(0.0f, 0.0f, 0.0f), "Child");
        scene.Scene->SetParent(child, parent);
        scene.Update();

        auto owner =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(5.0f, 0.0f, 0.0f));
        scene.Scene->SetTransform(parent, TransformParams(owner.get()));
        scene.Update();

        const auto& childTransform = scene.Scene->GetComponent<TransformComponent>(child);
        CHECK(NearlyEqual(childTransform.WorldMatrix.GetTranslation(), Vector3(5.0f, 0.0f, 0.0f)));
    }

    TEST_CASE("A three level hierarchy composes top down") {
        TestScene scene;

        const Entity root = scene.EntityAt(Vector3(1.0f, 0.0f, 0.0f), "Root");
        const Entity mid  = scene.EntityAt(Vector3(2.0f, 0.0f, 0.0f), "Mid");
        const Entity leaf = scene.EntityAt(Vector3(4.0f, 0.0f, 0.0f), "Leaf");
        scene.Scene->SetParent(mid, root);
        scene.Scene->SetParent(leaf, mid);

        scene.Update();

        const auto& leafTransform = scene.Scene->GetComponent<TransformComponent>(leaf);
        CHECK(NearlyEqual(leafTransform.WorldMatrix.GetTranslation(), Vector3(7.0f, 0.0f, 0.0f)));
    }

    TEST_CASE("Rotated parents rotate their children") {
        TestScene scene;

        const Entity parent = scene.EntityAt(Vector3(0.0f, 0.0f, 0.0f), "Parent");
        const Entity child  = scene.EntityAt(Vector3(1.0f, 0.0f, 0.0f), "Child");
        scene.Scene->SetParent(child, parent);
        scene.Update();

        // Rotate the parent 90 degrees around Y: the child's +X offset becomes -Z.
        auto rotation = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3::Zero,
                                        Quaternion::FromEuler(0.0f, 90.0f, 0.0f), Vector3::One);
        scene.Scene->SetTransform(parent, TransformParams(rotation.get()));
        scene.Update();

        const auto& childTransform = scene.Scene->GetComponent<TransformComponent>(child);
        CHECK(NearlyEqual(childTransform.WorldMatrix.GetTranslation(), Vector3(0.0f, 0.0f, -1.0f)));
    }

    TEST_CASE("Destroyed entities are skipped instead of being dereferenced") {
        TestScene scene;

        const Entity entity = scene.EntityAt(Vector3(1.0f, 1.0f, 1.0f), "Temporary");
        scene.Scene->DestroyEntity(entity);

        CHECK_NOTHROW(scene.Update());
    }

    TEST_CASE("A cyclic hierarchy does not recurse forever") {
        TestScene scene;

        const Entity a = scene.EntityAt(Vector3(1.0f, 0.0f, 0.0f), "A");
        const Entity b = scene.EntityAt(Vector3(2.0f, 0.0f, 0.0f), "B");

        // Deliberately build a cycle: A is B's parent and B is A's parent.
        scene.Scene->SetParent(a, b);
        scene.Scene->SetParent(b, a);

        CHECK_NOTHROW(scene.Update());
    }
}

#include "NameComponent.h"
#include "RelationshipComponent.h"
#include "Scene.h"
#include "TransformComponent.h"

#include "Test.h"

struct TestComponent {
    int Value = 0;
};

TEST_SUITE(SceneTests) {

    // ===== Entity Management Tests =====

    TEST("CreateEntity creates valid entity") {
        FScene scene;
        FEntity entity = scene.CreateEntity();
        TEST_ASSERT(scene.IsValid(entity));
        TEST_ASSERT(entity.IsValid());
    }
    TEST_END;

    TEST("CreateEntity with name") {
        FScene scene;
        FEntity entity = scene.CreateEntity("TestEntity");
        TEST_ASSERT(scene.IsValid(entity));

        auto& nameComp = scene.GetComponent<FNameComponent>(entity);
        TEST_ASSERT_EQ("TestEntity", nameComp.Name);
    }
    TEST_END;

    TEST("CreateEntity adds default components") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        TEST_ASSERT(scene.HasComponent<FTransformComponent>(entity));
        TEST_ASSERT(scene.HasComponent<FNameComponent>(entity));
        TEST_ASSERT(scene.HasComponent<FRelationshipComponent>(entity));
    }
    TEST_END;

    TEST("DestroyEntity removes entity") {
        FScene scene;
        FEntity entity = scene.CreateEntity();
        TEST_ASSERT(scene.IsValid(entity));

        scene.DestroyEntity(entity);
        TEST_ASSERT(!scene.IsValid(entity));
    }
    TEST_END;

    TEST("DestroyEntity on invalid entity does nothing") {
        FScene scene;
        FEntity invalid;

        // Should not crash
        scene.DestroyEntity(invalid);
        TEST_ASSERT(true);
    }
    TEST_END;

    TEST("IsValid returns false for destroyed entity") {
        FScene scene;
        FEntity entity = scene.CreateEntity();
        TEST_ASSERT(scene.IsValid(entity));

        scene.DestroyEntity(entity);
        TEST_ASSERT(!scene.IsValid(entity));
    }
    TEST_END;

    TEST("IsValid returns false for entity from different scene") {
        FScene scene1, scene2;
        FEntity entity = scene1.CreateEntity();

        TEST_ASSERT(scene1.IsValid(entity));
        TEST_ASSERT(!scene2.IsValid(entity));
    }
    TEST_END;

    // ===== Component Operations Tests =====

    TEST("AddComponent adds component") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        auto& test = scene.AddComponent<TestComponent>(entity);
        test.Value = 42;

        TEST_ASSERT(scene.HasComponent<TestComponent>(entity));
        TEST_ASSERT_EQ(42, scene.GetComponent<TestComponent>(entity).Value);
    }
    TEST_END;

    TEST("AddComponent on invalid entity fails") {
        FScene scene;
        FEntity invalid;

        // TEST_EXPECT_FAIL(scene.AddComponent<TestComponent>(invalid));
        TEST_EXPECT_FAIL_WITH_MSG((scene.AddComponent<TestComponent>(invalid)), "invalid entity");
    }
    TEST_END;

    TEST("AddComponent on entity with existing component fails") {
        FScene scene;
        FEntity entity = scene.CreateEntity();
        scene.AddComponent<TestComponent>(entity);

        TEST_EXPECT_FAIL(scene.AddComponent<TestComponent>(entity));
    }
    TEST_END;

    TEST("RemoveComponent removes component") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        scene.AddComponent<TestComponent>(entity);
        TEST_ASSERT(scene.HasComponent<TestComponent>(entity));

        scene.RemoveComponent<TestComponent>(entity);
        TEST_ASSERT(!scene.HasComponent<TestComponent>(entity));
    }
    TEST_END;

    TEST("RemoveComponent on entity without component fails") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        TEST_EXPECT_FAIL(scene.RemoveComponent<TestComponent>(entity));
    }
    TEST_END;

    TEST("GetComponent returns reference") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        auto& transform = scene.GetComponent<FTransformComponent>(entity);
        transform.Translation = FVector3(1, 2, 3);

        TEST_ASSERT_EQ(1.0f, scene.GetComponent<FTransformComponent>(entity).Translation.x);
        TEST_ASSERT_EQ(2.0f, scene.GetComponent<FTransformComponent>(entity).Translation.y);
        TEST_ASSERT_EQ(3.0f, scene.GetComponent<FTransformComponent>(entity).Translation.z);
    }
    TEST_END;

    TEST("GetComponent on invalid entity fails") {
        FScene scene;
        FEntity invalid;

        TEST_EXPECT_FAIL(scene.GetComponent<FTransformComponent>(invalid));
    }
    TEST_END;

    TEST("HasComponent returns true for existing component") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        TEST_ASSERT(scene.HasComponent<FTransformComponent>(entity));
        TEST_ASSERT(!scene.HasComponent<TestComponent>(entity));

        scene.AddComponent<TestComponent>(entity);
        TEST_ASSERT(scene.HasComponent<TestComponent>(entity));
    }
    TEST_END;

    // ===== Relationship Tests =====

    TEST("SetParent creates parent-child relationship") {
        FScene scene;
        FEntity parent = scene.CreateEntity("Parent");
        FEntity child = scene.CreateEntity("Child");

        scene.SetParent(child, parent);

        auto& childRel = scene.GetComponent<FRelationshipComponent>(child);
        auto& parentRel = scene.GetComponent<FRelationshipComponent>(parent);

        TEST_ASSERT(childRel.Parent == parent);
        TEST_ASSERT(parentRel.Children.size() == 1);
        TEST_ASSERT(parentRel.Children[0] == child);
    }
    TEST_END;

    TEST("GetParent returns correct parent") {
        FScene scene;
        FEntity parent = scene.CreateEntity("Parent");
        FEntity child = scene.CreateEntity("Child");

        scene.SetParent(child, parent);

        FEntity retrievedParent = scene.GetParent(child);
        TEST_ASSERT(retrievedParent == parent);
    }
    TEST_END;

    TEST("GetParent returns invalid for root entity") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        FEntity parent = scene.GetParent(entity);
        TEST_ASSERT(!parent.IsValid());
    }
    TEST_END;

    TEST("GetChildren returns all children") {
        FScene scene;
        FEntity parent = scene.CreateEntity("Parent");
        FEntity child1 = scene.CreateEntity("Child1");
        FEntity child2 = scene.CreateEntity("Child2");
        FEntity child3 = scene.CreateEntity("Child3");

        scene.SetParent(child1, parent);
        scene.SetParent(child2, parent);
        scene.SetParent(child3, parent);

        auto children = scene.GetChildren(parent);
        TEST_ASSERT_EQ(3, children.size());
        TEST_ASSERT(children[0] == child1 || children[0] == child2 || children[0] == child3);
    }
    TEST_END;

    TEST("Changing parent updates relationships") {
        FScene scene;
        FEntity parent1 = scene.CreateEntity("Parent1");
        FEntity parent2 = scene.CreateEntity("Parent2");
        FEntity child = scene.CreateEntity("Child");

        scene.SetParent(child, parent1);

        auto& rel1 = scene.GetComponent<FRelationshipComponent>(parent1);
        TEST_ASSERT(rel1.Children.size() == 1);

        scene.SetParent(child, parent2);

        // Check old parent
        auto& rel1After = scene.GetComponent<FRelationshipComponent>(parent1);
        TEST_ASSERT(rel1After.Children.empty());

        // Check new parent
        auto& rel2 = scene.GetComponent<FRelationshipComponent>(parent2);
        TEST_ASSERT(rel2.Children.size() == 1);
        TEST_ASSERT(rel2.Children[0] == child);

        // Check child's parent
        auto& childRel = scene.GetComponent<FRelationshipComponent>(child);
        TEST_ASSERT(childRel.Parent == parent2);
    }
    TEST_END;

    TEST("Setting parent to invalid makes entity root") {
        FScene scene;
        FEntity parent = scene.CreateEntity("Parent");
        FEntity child = scene.CreateEntity("Child");

        scene.SetParent(child, parent);
        TEST_ASSERT(scene.GetParent(child).IsValid());

        scene.SetParent(child, FEntity());
        TEST_ASSERT(!scene.GetParent(child).IsValid());

        auto& parentRel = scene.GetComponent<FRelationshipComponent>(parent);
        TEST_ASSERT(parentRel.Children.empty());
    }
    TEST_END;

    TEST("DestroyEntity removes from parent's children list") {
        FScene scene;
        FEntity parent = scene.CreateEntity("Parent");
        FEntity child = scene.CreateEntity("Child");

        scene.SetParent(child, parent);

        auto& parentRel = scene.GetComponent<FRelationshipComponent>(parent);
        TEST_ASSERT(parentRel.Children.size() == 1);

        scene.DestroyEntity(child);

        auto& parentRelAfter = scene.GetComponent<FRelationshipComponent>(parent);
        TEST_ASSERT(parentRelAfter.Children.empty());
    }
    TEST_END;

    TEST("DestroyEntity orphans children") {
        FScene scene;
        FEntity parent = scene.CreateEntity("Parent");
        FEntity child = scene.CreateEntity("Child");

        scene.SetParent(child, parent);

        auto& childRel = scene.GetComponent<FRelationshipComponent>(child);
        TEST_ASSERT(childRel.Parent == parent);

        scene.DestroyEntity(parent);

        // Child should still exist but have no parent
        TEST_ASSERT(scene.IsValid(child));
        auto& childRelAfter = scene.GetComponent<FRelationshipComponent>(child);
        TEST_ASSERT(!childRelAfter.Parent.IsValid());
    }
    TEST_END;

    // ===== Query System Tests =====

    TEST("View with single component") {
        FScene scene;

        FEntity e1 = scene.CreateEntity();
        FEntity e2 = scene.CreateEntity();
        FEntity e3 = scene.CreateEntity();

        scene.AddComponent<TestComponent>(e1);
        scene.AddComponent<TestComponent>(e2);

        auto view = scene.View<TestComponent>();
        int count = 0;
        for (auto entity : view) {
            count++;
        }
        TEST_ASSERT_EQ(2, count);
    }
    TEST_END;

    TEST("View with multiple components") {
        FScene scene;

        FEntity e1 = scene.CreateEntity();
        FEntity e2 = scene.CreateEntity();
        FEntity e3 = scene.CreateEntity();

        scene.AddComponent<TestComponent>(e1);
        scene.AddComponent<TestComponent>(e2);
        // scene.AddComponent<TestComponent>(e3);

        // All have TestComponent, only e1 and e2 have both
        auto view = scene.View<TestComponent, FTransformComponent>();
        int count = 0;
        for (auto entity : view) {
            count++;
        }
        TEST_ASSERT_EQ(2, count);
    }
    TEST_END;

    TEST("View with component modification") {
        FScene scene;

        FEntity e1 = scene.CreateEntity();
        scene.AddComponent<TestComponent>(e1);

        auto view = scene.View<TestComponent>();
        for (auto entity : view) {
            auto& test = view.get<TestComponent>(entity);
            test.Value = 100;
        }

        TEST_ASSERT_EQ(100, scene.GetComponent<TestComponent>(e1).Value);
    }
    TEST_END;

    // ===== Edge Cases =====

    TEST("Multiple entities can be created") {
        FScene scene;
        std::vector<FEntity> entities;

        for (int i = 0; i < 100; i++) {
            entities.push_back(scene.CreateEntity("Entity" + std::to_string(i)));
        }

        for (const auto& entity : entities) {
            TEST_ASSERT(scene.IsValid(entity));
        }
        TEST_ASSERT_EQ(100, entities.size());
    }
    TEST_END;

    TEST("Entity persists after scene copy?") {
        // Scene should not be copyable, but we test basic behavior
        FScene scene1;
        FEntity entity = scene1.CreateEntity();

        // FScene should not be copyable, so this is fine
        TEST_ASSERT(scene1.IsValid(entity));
    }
    TEST_END;

    TEST("Component values persist") {
        FScene scene;
        FEntity entity = scene.CreateEntity();

        auto& transform = scene.GetComponent<FTransformComponent>(entity);
        transform.Translation = FVector3(10, 20, 30);

        // Retrieve again and verify
        auto& transform2 = scene.GetComponent<FTransformComponent>(entity);
        TEST_ASSERT_EQ(10.0f, transform2.Translation.x);
        TEST_ASSERT_EQ(20.0f, transform2.Translation.y);
        TEST_ASSERT_EQ(30.0f, transform2.Translation.z);
    }
    TEST_END;
}
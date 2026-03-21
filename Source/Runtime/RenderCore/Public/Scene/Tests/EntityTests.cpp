#include "Entity.h"

#include "Test.h"

#include <sstream>
#include <unordered_set>

TEST_SUITE(EntityTests) {

    TEST("Default constructor creates invalid entity") {
        FEntity entity;
        TEST_ASSERT(!entity.IsValid());
        TEST_ASSERT(!static_cast<bool>(entity));
        TEST_ASSERT_EQ(static_cast<uint32_t>(entt::null),
                       static_cast<uint32_t>(entity.GetHandle()));
    }
    TEST_END;

    TEST("Constructor with handle creates valid entity") {
        entt::entity handle = static_cast<entt::entity>(42);
        FEntity entity(handle);
        TEST_ASSERT(entity.IsValid());
        TEST_ASSERT(static_cast<bool>(entity));
        TEST_ASSERT_EQ(handle, entity.GetHandle());
    }
    TEST_END;

    TEST("Constructor with uint32_t creates valid entity") {
        FEntity entity(42u);
        TEST_ASSERT(entity.IsValid());
        TEST_ASSERT_EQ(static_cast<entt::entity>(42), entity.GetHandle());
    }
    TEST_END;

    TEST("Equality operator") {
        FEntity entity1(static_cast<entt::entity>(10));
        FEntity entity2(static_cast<entt::entity>(10));
        FEntity entity3(static_cast<entt::entity>(20));

        TEST_ASSERT(entity1 == entity2);
        TEST_ASSERT(!(entity1 == entity3));
        TEST_ASSERT(entity1 != entity3);
    }
    TEST_END;

    TEST("Conversion to entt::entity") {
        entt::entity raw = static_cast<entt::entity>(123);
        FEntity entity(raw);
        entt::entity converted = entity;
        TEST_ASSERT_EQ(raw, converted);
    }
    TEST_END;

    TEST("Conversion to uint32_t") {
        FEntity entity(static_cast<entt::entity>(456));
        uint32_t value = entity;
        TEST_ASSERT_EQ(456u, value);
    }
    TEST_END;

    TEST("Entity can be stored in unordered_set") {
        std::unordered_set<FEntity> entities;

        FEntity e1(static_cast<entt::entity>(1));
        FEntity e2(static_cast<entt::entity>(2));

        entities.insert(e1);
        entities.insert(e2);

        TEST_ASSERT_EQ(2, entities.size());
        TEST_ASSERT(entities.find(e1) != entities.end());
        TEST_ASSERT(entities.find(e2) != entities.end());
    }
    TEST_END;

    TEST("ToString for valid entity") {
        FEntity valid(static_cast<entt::entity>(42));
        TEST_ASSERT_EQ("Entity[42]", valid.ToString());
    }
    TEST_END;

    TEST("ToString for invalid entity") {
        FEntity invalid;
        TEST_ASSERT_EQ("Entity[invalid]", invalid.ToString());
    }
    TEST_END;

    TEST("Ostream output") {
        FEntity valid(static_cast<entt::entity>(42));
        std::stringstream ss;
        ss << valid;
        TEST_ASSERT_EQ("Entity[42]", ss.str());
    }
    TEST_END;

    TEST("Fmt formatting") {
        FEntity valid(static_cast<entt::entity>(42));
        std::string result = fmt::format("{}", valid);
        TEST_ASSERT_EQ("Entity[42]", result);
    }
    TEST_END;

    TEST("Large handle values") {
        entt::entity large = static_cast<entt::entity>(0xFFFFFFFF);
        FEntity entity(large);
        TEST_ASSERT_EQ(large, entity.GetHandle());
        TEST_ASSERT_EQ(0xFFFFFFFFu, static_cast<uint32_t>(entity));
    }
    TEST_END;

    TEST("Max valid handle value") {
        entt::entity large = static_cast<entt::entity>(0x00FFFFFF);
        FEntity entity(large);
        TEST_ASSERT_EQ(large, entity.GetHandle());
    }
    TEST_END;

    TEST("Multiple entities with same handle are equal") {
        entt::entity handle = static_cast<entt::entity>(100);
        FEntity e1(handle);
        FEntity e2(handle);
        TEST_ASSERT(e1 == e2);
    }
    TEST_END;
}
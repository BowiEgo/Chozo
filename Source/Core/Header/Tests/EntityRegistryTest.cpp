#include <Core/EntityRegistry/EntityRegistry.hpp>

#include <doctest/doctest.h>

using namespace CZ;

namespace {

struct DummyObject {
    int Value = 0;
};

struct DummyPolicy {
    template <typename T, typename... Args> T* Allocate(Args&&... args) {
        return new T(std::forward<Args>(args)...);
    }

    template <typename T> void Deallocate(T* ptr) { delete ptr; }

    template <typename T> T* Get(T* ptr) { return ptr; }
};

class DummyRegistry : public EntityRegistry<DummyObject, DummyPolicy> {
public:
    void Init() override {}
    void Shutdown() override {}
};

} // namespace

TEST_SUITE("EntityRegistry") {

    TEST_CASE("Create/Get/Destroy keep the handle consistent") {
        DummyRegistry registry;

        Handle<DummyObject> handle = registry.Create();
        REQUIRE(static_cast<bool>(handle));
        CHECK(registry.Get(handle) == InternalHandleReader::Unwrap(handle));

        registry.Destroy(handle);
        CHECK_FALSE(static_cast<bool>(handle));
    }

    TEST_CASE("Destroying a default constructed handle is a no-op") {
        DummyRegistry registry;

        Handle<DummyObject> handle;
        CHECK_NOTHROW(registry.Destroy(handle));
        CHECK_FALSE(static_cast<bool>(handle));
    }
}

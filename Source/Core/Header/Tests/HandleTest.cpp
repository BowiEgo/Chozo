#include <Core/Header/Handle.hpp>

#include <doctest/doctest.h>

using namespace CZ;

namespace {

struct DummyObject {
    int Value = 0;
};

} // namespace

TEST_SUITE("Handle") {

    TEST_CASE("Default constructed handles are null") {
        Handle<DummyObject> handle;

        CHECK(InternalHandleReader::Unwrap(handle) == nullptr);
        CHECK_FALSE(static_cast<bool>(handle));
    }

    TEST_CASE("Handles constructed from a pointer expose it") {
        DummyObject object;
        Handle<DummyObject> handle(&object);

        CHECK(InternalHandleReader::Unwrap(handle) == &object);
        CHECK(static_cast<bool>(handle));
    }
}

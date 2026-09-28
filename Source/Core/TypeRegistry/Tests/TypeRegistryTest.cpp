#include <Core/TypeRegistry/TypeRegistry.hpp>

#include <doctest/doctest.h>

using namespace CZ;

namespace {

// The registry is a process-wide singleton, so every case registers uniquely named types.
int s_TypeCounter = 0;

std::string UniqueName(const char* prefix) {
    return std::string(prefix) + "_" + std::to_string(s_TypeCounter++);
}

} // namespace

TEST_SUITE("TypeRegister") {

    TEST_CASE("Registration is idempotent per name") {
        auto& registry         = TypeRegister::Get();
        const std::string name = UniqueName("TestType");

        const Type first  = registry.RegisterType(name, true, TypeCategory::Node);
        const Type second = registry.RegisterType(name); // same name again

        CHECK_EQ(first, second);
        CHECK(registry.IsTypeValid(first));
    }

    TEST_CASE("Category flags classify the type") {
        auto& registry = TypeRegister::Get();

        const Type nodeType = registry.RegisterType(UniqueName("Node"), true, TypeCategory::Node);
        const Type meshType = registry.RegisterType(UniqueName("Mesh"), true, TypeCategory::Mesh);
        const Type lightType =
            registry.RegisterType(UniqueName("Light"), true, TypeCategory::Light);
        const Type matType =
            registry.RegisterType(UniqueName("Material"), true, TypeCategory::Material);
        const Type mixedType = registry.RegisterType(UniqueName("Mixed"), true,
                                                     TypeCategory::Node | TypeCategory::Mesh);

        CHECK(registry.IsNodeType(nodeType));
        CHECK_FALSE(registry.IsMeshType(nodeType));

        CHECK(registry.IsMeshType(meshType));
        CHECK_FALSE(registry.IsLightType(meshType)); // regression: IsLightType used the mesh mask

        CHECK(registry.IsLightType(lightType));
        CHECK_FALSE(registry.IsNodeType(lightType));

        CHECK(registry.IsMaterialType(matType));

        CHECK(registry.IsNodeType(mixedType));
        CHECK(registry.IsMeshType(mixedType));
    }

    TEST_CASE("Unknown names resolve to an invalid type") {
        auto& registry     = TypeRegister::Get();
        const Type unknown = registry.GetType(UniqueName("DoesNotExist"));

        CHECK_FALSE(registry.IsTypeValid(unknown));
        CHECK(registry.GetTypeInfo(unknown) == nullptr);
    }

    TEST_CASE("Type info carries the registered metadata") {
        auto& registry         = TypeRegister::Get();
        const std::string name = UniqueName("Info");

        const Type type      = registry.RegisterType(name, true, TypeCategory::Node);
        const TypeInfo* info = registry.GetTypeInfo(type);

        REQUIRE(info != nullptr);
        CHECK_EQ(info->Name, name);
        CHECK_EQ(info->Index, type);
        CHECK(info->bBuiltin);
        CHECK_EQ(info->CategoryFlags, TypeCategory::Node);
    }

    TEST_CASE("Masks and names stay in sync") {
        auto& registry         = TypeRegister::Get();
        const std::string name = UniqueName("Masked");

        const Type type = registry.RegisterType(name, true, TypeCategory::Mesh);

        CHECK(registry.GetMask(type).Test(type));
        CHECK(registry.GetMeshMask().Test(type));
        CHECK(registry.GetAllMask().Test(type));
        CHECK_EQ(registry.TypeMaskToString(registry.GetMask(type)), name);
    }

    TEST_CASE("Unknown names are reported when building a mask") {
        auto& registry = TypeRegister::Get();
        CHECK_EQ(registry.TypeMaskToString(TypeMask()), "None");
    }
}

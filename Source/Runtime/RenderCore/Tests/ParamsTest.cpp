// Parameter objects: value semantics, the visitor interface that drives the properties panel,
// and the factory used to clone/default them per type.
#include <any>

#include <Runtime/RenderCore/Components/TransformParams.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>
#include <Runtime/RenderCore/Params.hpp>
#include <Runtime/RenderCore/ProceduralMesh/CubeParamsObj.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <string>
#include <vector>

using namespace CZ;

namespace {

/// Minimal visitor that records which fields it was shown, in order.
struct RecordingVisitor : public ParamsVisitor {
    std::vector<std::string> Names;

    void Visit(float&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(double&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(int32_t&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(uint32_t&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(int64_t&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(uint64_t&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(bool&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(std::string&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(Vector2&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(Vector3&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(Vector4&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(Quaternion&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
    void Visit(AssetHandle&, const std::string& name, ParamControllerConfig) override {
        Names.push_back(name);
    }
};

struct ConstRecordingVisitor : public ConstParamsVisitor {
    // The visitor interface is const, so the collected names have to be mutable.
    mutable std::vector<std::string> Names;

    void Visit(const float&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const double&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const int32_t&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const uint32_t&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const int64_t&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const uint64_t&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const bool&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const std::string&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const Vector2&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const Vector3&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const Vector4&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const Quaternion&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
    void Visit(const AssetHandle&, const std::string& name, ParamControllerConfig) const override {
        Names.push_back(name);
    }
};

bool Contains(const std::vector<std::string>& names, const std::string& needle) {
    return std::find(names.begin(), names.end(), needle) != names.end();
}

} // namespace

TEST_SUITE("Params") {

    TEST_CASE("Transform params compare by value, not by pointer") {
        auto first =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(1.0f, 2.0f, 3.0f));
        auto second =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(1.0f, 2.0f, 3.0f));
        auto other =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(9.0f, 2.0f, 3.0f));

        CHECK_FALSE(first.get() == second.get());
        CHECK(*first == second.get());
        CHECK_FALSE(*first == other.get());
    }

    TEST_CASE("Value assignment copies every field") {
        auto source =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(1.0f, 2.0f, 3.0f),
                            Quaternion::FromEuler(10.0f, 20.0f, 30.0f), Vector3(2.0f, 2.0f, 2.0f));
        auto target = CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3::Zero);

        *target = *source;

        CHECK(*target == source.get());
        CHECK_EQ(target->Scale, Vector3(2.0f, 2.0f, 2.0f));
    }

    TEST_CASE("The visitor is shown every field by name") {
        auto params =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(1.0f, 2.0f, 3.0f));

        RecordingVisitor visitor;
        params->Accept(visitor);

        REQUIRE_EQ(visitor.Names.size(), 3u);
        CHECK(Contains(visitor.Names, "Translation"));
        CHECK(Contains(visitor.Names, "Rotation"));
        CHECK(Contains(visitor.Names, "Scale"));
    }

    TEST_CASE("The read-only visitor is shown the same fields") {
        auto params =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(1.0f, 2.0f, 3.0f));

        ConstRecordingVisitor visitor;
        params->Accept(visitor);

        REQUIRE_EQ(visitor.Names.size(), 3u);
        CHECK(Contains(visitor.Names, "Translation"));
    }

    TEST_CASE("Named lookup returns the requested field") {
        auto params =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(4.0f, 5.0f, 6.0f));

        CHECK(std::any_cast<Vector3>(params->GetParamValue("Translation")) ==
              Vector3(4.0f, 5.0f, 6.0f));
        CHECK_EQ(params->GetTypeName(), std::string("Transform"));
    }

    TEST_CASE("The factory creates defaults and deep copies") {
        TParamsFactory<TransformParamsObj> factory;

        CHECK_EQ(factory.GetTypeName(), std::string("Transform"));

        Scope<Params> created = factory.CreateDefault();
        REQUIRE(created != nullptr);

        auto original =
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(7.0f, 8.0f, 9.0f));

        Scope<Params> clone = factory.Clone(original.get());
        REQUIRE(clone != nullptr);
        CHECK(*static_cast<TransformParamsObj*>(clone.get()) == original.get());

        // A clone is an independent object.
        static_cast<TransformParamsObj*>(clone.get())->Translation = Vector3::Zero;
        CHECK(*original ==
              CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(7.0f, 8.0f, 9.0f))
                  .get());
    }

    TEST_CASE("Mesh params expose their fields through the same interface") {
        auto cube = CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, CubeParamsObj, 2.0f, 3.0f, 4.0f, 2, 2, 2);

        CHECK_EQ(cube->GetTypeName(), std::string("Cube"));
        CHECK_GT(cube->GetParamCount(), 0u);
        CHECK_FALSE(cube->GetParamName(0).empty());

        RecordingVisitor visitor;
        cube->Accept(visitor);

        CHECK(Contains(visitor.Names, "Width"));
        CHECK(Contains(visitor.Names, "Material"));

        // Cloning keeps the values but produces a distinct object.
        Scope<MeshParamsObj> clone(cube->Clone());
        REQUIRE(clone != nullptr);
        CHECK(*clone == cube.get());
        CHECK_EQ(static_cast<CubeParamsObj*>(clone.get())->Width, 2.0f);
    }
}

#pragma once
#include <any>
#include <cstddef>
#include <utility>
#include <vector>

#include <Runtime/RenderCore/Params.hpp>

namespace CZ {

/// Captures a parameter object's fields in visitor order and restores them later.
///
/// Params exposes GetParamValue by name but no symmetric setter, while ParamsVisitor::Visit
/// receives the field by mutable reference. Snapshotting through the visitor therefore needs no
/// per-type glue and keeps working for any object whose fields are declared with PARAMS_LIST --
/// which is every procedural mesh, the transform, and anything added later.
///
/// Capture and restore walk the same order, so the values line up without naming fields.
struct ParamsSnapshot {
    std::vector<std::any> Values;

    bool Empty() const { return Values.empty(); }
};

class CaptureParamsVisitor final : public ParamsVisitor {
public:
    std::vector<std::any> Values;

    CaptureParamsVisitor() = default;

    void Visit(float& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(double& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(int32_t& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(uint32_t& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(int64_t& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(uint64_t& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(bool& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(std::string& value, const std::string& name, ) override {
        Values.emplace_back(value);
    }
    void Visit(Vector2& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(Vector3& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(Vector4& value, const std::string& name, ) override { Values.emplace_back(value); }
    void Visit(Quaternion& value, const std::string& name, ) override {
        Values.emplace_back(value);
    }
    void Visit(AssetHandle& value, const std::string& name, ) override {
        Values.emplace_back(value);
    }
};

inline ParamsSnapshot CaptureParams(Params& params) {
    CaptureParamsVisitor visitor;
    params.Accept(visitor);
    return ParamsSnapshot{ {
        std::move(visitor.Values),
    } };
}

inline void RestoreParams(Params& params, const ParamsSnapshot& snapshot) {
    RestoreParamsVisitor visitor(snapshot.Values);
    params.Accept(visitor);
}

} // namespace CZ

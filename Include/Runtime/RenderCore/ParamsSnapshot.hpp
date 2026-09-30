#pragma once

#include <any>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <Runtime/RenderCore/Params.hpp>

namespace CZ {

/// Captures a parameter object's fields in visitor order and restores them later.
///
/// Params offers GetParamValue by name but no symmetric setter, while ParamsVisitor::Visit receives
/// each field by mutable reference, so snapshotting through the visitor needs no per-type glue and
/// keeps working for anything declared with PARAMS_LIST -- every procedural mesh, the transform,
/// and whatever is added later. Capture and restore walk the same order, so values line up by
/// position.
struct ParamsSnapshot {
    std::vector<std::any> Values;
    /// Fields this snapshot covers, in the same order as Values. Empty means "every field", which
    /// is what a whole-object snapshot is; a masked snapshot lets a command restore only the fields
    /// an edit touched, so undoing a translation change no longer rewinds rotation and scale.
    std::vector<std::size_t> Indices;

    bool Empty() const { return Values.empty(); }
};

class CaptureParamsVisitor final : public ParamsVisitor {
public:
    std::vector<std::any> Values;

    void Visit(float& value, const std::string& name, const ParamControllerConfig config) override;
    void Visit(double& value, const std::string& name, const ParamControllerConfig config) override;
    void Visit(int32_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(uint32_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(int64_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(uint64_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(bool& value, const std::string& name, const ParamControllerConfig config) override;
    void Visit(std::string& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Vector2& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Vector3& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Vector4& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Quaternion& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(AssetHandle& value, const std::string& name,
               const ParamControllerConfig config) override;
};

class RestoreParamsVisitor final : public ParamsVisitor {
public:
    explicit RestoreParamsVisitor(const std::vector<std::any>& values) : Values(values) {}

    const std::vector<std::any>& Values;
    std::size_t Index = 0;

    void Visit(float& value, const std::string& name, const ParamControllerConfig config) override;
    void Visit(double& value, const std::string& name, const ParamControllerConfig config) override;
    void Visit(int32_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(uint32_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(int64_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(uint64_t& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(bool& value, const std::string& name, const ParamControllerConfig config) override;
    void Visit(std::string& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Vector2& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Vector3& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Vector4& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(Quaternion& value, const std::string& name,
               const ParamControllerConfig config) override;
    void Visit(AssetHandle& value, const std::string& name,
               const ParamControllerConfig config) override;
};

inline void CaptureParamsVisitor::Visit(float& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(double& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(int32_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(uint32_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(int64_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(uint64_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(bool& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(std::string& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(Vector2& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(Vector3& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(Vector4& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(Quaternion& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}
inline void CaptureParamsVisitor::Visit(AssetHandle& value, const std::string& name,
                                        const ParamControllerConfig config) {
    Values.emplace_back(value);
}

inline void RestoreParamsVisitor::Visit(float& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(double& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(int32_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(uint32_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(int64_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(uint64_t& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(bool& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(std::string& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(Vector2& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(Vector3& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(Vector4& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(Quaternion& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}
inline void RestoreParamsVisitor::Visit(AssetHandle& value, const std::string& name,
                                        const ParamControllerConfig config) {
    if (Index < Values.size()) {
        if (const auto* stored =
                std::any_cast<std::remove_cv_t<std::remove_reference_t<decltype(value)>>>(
                    &Values[Index])) {
            value = *stored;
        }
    }
    ++Index;
}

inline ParamsSnapshot CaptureParams(Params& params) {
    CaptureParamsVisitor visitor;
    params.Accept(visitor);
    return ParamsSnapshot{ std::move(visitor.Values) };
}

inline void RestoreParams(Params& params, const ParamsSnapshot& snapshot) {
    RestoreParamsVisitor visitor(snapshot.Values);
    params.Accept(visitor);
}

/// Full snapshot restricted to the given fields (values keep the order of `indices`).
inline ParamsSnapshot MaskSnapshot(const ParamsSnapshot& full,
                                   const std::vector<std::size_t>& indices) {
    ParamsSnapshot masked;
    masked.Indices = indices;
    for (const std::size_t index : indices) {
        if (index < full.Values.size()) {
            masked.Values.push_back(full.Values[index]);
        }
    }
    return masked;
}

/// Applies only the fields a masked snapshot names; everything else keeps its current value, which
/// is what makes an undo touch just the fields the edit changed.
inline void RestoreParamsMasked(Params& params, const ParamsSnapshot& masked) {
    if (masked.Indices.empty()) {
        RestoreParams(params, masked);
        return;
    }

    ParamsSnapshot full = CaptureParams(params);
    for (std::size_t i = 0; i < masked.Indices.size() && i < masked.Values.size(); ++i) {
        const std::size_t index = masked.Indices[i];
        if (index < full.Values.size()) {
            full.Values[index] = masked.Values[i];
        }
    }
    RestoreParams(params, full);
}

} // namespace CZ

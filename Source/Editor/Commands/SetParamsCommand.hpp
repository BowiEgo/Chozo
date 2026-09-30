#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <Core/Command/Command.hpp>
#include <Runtime/RenderCore/Components/TransformParams.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>
#include <Runtime/RenderCore/ParamsSnapshot.hpp>

#include "../EditorNode/EditorNode.hpp"

namespace CZ {

/// Any parameter edit, as a before/after snapshot of the whole parameter object.
///
/// The command holds the *node*, not a Params pointer. Editing a mesh parameter marks the component
/// dirty, SceneObj::Update rebuilds the mesh, and the node's parameter object can be replaced in
/// the process -- a command holding the old address would write into an orphan and undo would
/// silently do nothing, which is the "only the last few steps undo" symptom. Resolving the
/// parameters on every Apply/Undo keeps the command valid for the node's whole life.
class SetParamsCommand final : public Command {
public:
    enum class Target : uint8_t { Transform, Mesh };

    SetParamsCommand(EditorNode* node, Target target, ParamsSnapshot before, ParamsSnapshot after,
                     std::string label, std::function<void()> onChanged)
        : m_Node(node), m_Target(target), m_Before(std::move(before)), m_After(std::move(after)),
          m_Label(std::move(label)), m_OnChanged(std::move(onChanged)) {}

    void Apply() override { Restore(m_After); }
    void Undo() override { Restore(m_Before); }

    std::string_view Label() const override { return m_Label; }

private:
    Params* Resolve() const {
        if (!m_Node) {
            return nullptr;
        }
        if (m_Target == Target::Mesh) {
            return m_Node->GetMeshParams().As<MeshParamsObj>();
        }
        return m_Node->GetTransformParams().Unwrap();
    }

    void Restore(const ParamsSnapshot& snapshot) {
        Params* params = Resolve();
        if (!params) {
            return;
        }
        RestoreParams(*params, snapshot);
        if (m_OnChanged) {
            m_OnChanged();
        }
    }

    EditorNode* m_Node = nullptr;
    Target m_Target    = Target::Mesh;
    ParamsSnapshot m_Before;
    ParamsSnapshot m_After;
    std::string m_Label;
    std::function<void()> m_OnChanged;
};

} // namespace CZ

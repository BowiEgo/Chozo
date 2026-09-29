#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <Core/Command/Command.hpp>
#include <Runtime/RenderCore/ParamsSnapshot.hpp>

namespace CZ {

/// Any parameter edit, as a before/after snapshot of the whole parameter object.
///
/// One class covers every case: a dragged field, a typed value and "reset to default" (where
/// `after` is the default snapshot). Resetting a field is therefore an ordinary undoable edit,
/// which is what Unity, Unreal and Blender all do, and no per-type glue is needed because
/// ParamsSnapshot walks the same visitor order PARAMS_LIST defines.
///
/// The command never touches the ECS: `onChanged` marks the node dirty, which is what makes the
/// sync bridge push the change and SceneObj::Update rebuild the mesh.
class SetParamsCommand final : public Command {
public:
    SetParamsCommand(Params* target, ParamsSnapshot before, ParamsSnapshot after, std::string label,
                     std::function<void()> onChanged)
        : m_Target(target), m_Before(std::move(before)), m_After(std::move(after)),
          m_Label(std::move(label)), m_OnChanged(std::move(onChanged)) {}

    void Apply() override { Restore(m_After); }
    void Undo() override { Restore(m_Before); }

    std::string_view Label() const override { return m_Label; }

private:
    void Restore(const ParamsSnapshot& snapshot) {
        if (!m_Target) {
            return;
        }
        RestoreParams(*m_Target, snapshot);
        if (m_OnChanged) {
            m_OnChanged();
        }
    }

    Params* m_Target = nullptr;
    ParamsSnapshot m_Before;
    ParamsSnapshot m_After;
    std::string m_Label;
    std::function<void()> m_OnChanged;
};

} // namespace CZ

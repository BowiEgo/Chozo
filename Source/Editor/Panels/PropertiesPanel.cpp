#include "../Commands/SetParamsCommand.hpp"
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <string>

#include "PropertiesPanel.hpp"

#include "../Widgets/PropControllers.hpp"

void PropertiesPanel::Draw(const char* title) {
    if (!m_IsOpen) return;

    if (!ImGui::Begin(title, &m_IsOpen)) {
        ImGui::End();
        return;
    }

    if (m_NodeTree) {
        auto node = m_NodeTree->GetSelectedNode();
        if (node) {
            DrawInfoProperties(node);
            DrawTransformProperties(node);
            DrawHDRIBackdropProperties(node);
            DrawMeshProperties(node);
        }
    }

    ImGui::End();
}

void PropertiesPanel::DrawComponentHeader(const std::string& name, bool bDefaultOpen,
                                          const DrawContentFunc& drawContentFunc) {
    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_AllowOverlap |
                                       ImGuiTreeNodeFlags_SpanAvailWidth |
                                       ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

    if (bDefaultOpen) treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    const float lineHeight        = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;

    ImGui::Separator();
    bool open =
        ImGui::TreeNodeEx((void*)std::hash<std::string>{}(name), treeNodeFlags, "%s", name.c_str());

    ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
    if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight })) {
    }

    if (open) {
        drawContentFunc();
        ImGui::TreePop();
    }
}

bool PropertiesPanel::DrawColumnProperties(const std::string& name, Params* params,
                                           const ParamsSnapshot* defaults) {
    bool valChanged      = false;
    // Consumed every frame, not only when a commit happens: undo and redo move the values without a
    // commit, and the baseline has to be dropped on that very frame or the next edit compares
    // against a state that no longer exists.
    const bool bApplying = m_Commands && m_Commands->ConsumeApplied();
    if (bApplying) {
        m_LastCommitted.clear();
    }
    if (constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable;
        ImGui::BeginTable("table", 2, flags)) {

        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        // Snapshotted around the whole draw: one command per finished gesture.
        EditorNode* selected = m_NodeTree ? m_NodeTree->GetSelectedNode() : nullptr;
        if (selected != m_LastCommittedNode) {
            m_LastCommitted.clear();
            m_LastCommittedNode = selected;
        }
        // The reference for the *command* is the last committed state, not this frame's value:
        // ImGui finishes a gesture one frame after the edit, so a snapshot taken here would already
        // contain it and the first command of a session would undo to itself.
        if (params && m_LastCommitted.find(name) == m_LastCommitted.end()) {
            m_LastCommitted.emplace(name, CaptureParams(*params));
        }
        const ParamsSnapshot beforeFull = params ? m_LastCommitted[name] : ParamsSnapshot{};

        EditorParamsVisitor visitor;
        visitor.SetDefaults(defaults);
        // Part of the gesture key: two sections are drawn in one frame and each numbers its own
        // fields from zero.
        visitor.SetSource(params);
        params->Accept(visitor);

        if (visitor.IsValueChanged()) {
            valChanged = true;
        }

        const bool bReset     = visitor.ConsumeResetRequest();
        const bool bCommitted = visitor.IsEditCommitted() || bReset;
        visitor.ResetEditCommitted();

        if (params && bCommitted && m_Commands && !bApplying) {
            // (never commit a restore)
            const std::string label =
                bReset ? fmt::format("Reset {}.{}", params->GetTypeName(), visitor.ResetLabel())
                       : fmt::format("Edit {}", params->GetTypeName());
            EditorNode* commandNode = m_NodeTree ? m_NodeTree->GetSelectedNode() : nullptr;
            // Only the fields the finished gesture touched; the mask is known once the draw is
            // done.
            const std::vector<size_t>& changedFields = visitor.CommittedFields();
            const ParamsSnapshot before              = MaskSnapshot(beforeFull, changedFields);
            const ParamsSnapshot after = MaskSnapshot(CaptureParams(*params), changedFields);
            const auto target          = (name == "Mesh") ? SetParamsCommand::Target::Mesh
                                                          : SetParamsCommand::Target::Transform;
            CZ_EDITOR_LOG(Warning, "push '{}' depth={} node={}", label, m_Commands->UndoDepth(),
                          static_cast<const void*>(commandNode));
            if (m_Commands && m_Commands->ConsumeApplied()) {
                // The values were moved by a command, so the baseline is stale: drop it and the
                // next edit captures the current state, which is what "undo, then edit, then undo"
                // needs.
                m_LastCommitted.clear();
            } else {
                m_LastCommitted[name] = CaptureParams(*params);
            }
            m_Commands->Execute(std::make_unique<SetParamsCommand>(
                commandNode, target, before, after, label, [this]() {
                    if (m_NodeTree) {
                        if (EditorNode* node = m_NodeTree->GetSelectedNode()) node->MarkDirty();
                    }
                }));
        }

        ImGui::EndTable();
    }

    return valChanged;
}

void PropertiesPanel::DrawInfoProperties(EditorNode* node) {
    if (!node) return;

    DrawComponentHeader("Info", true,
                        [this, node]() { ImGui::Text("%s", node->GetName().c_str()); });
}

void PropertiesPanel::DrawTransformProperties(EditorNode* node) {
    if (!node) return;
    if (!node->HasTransform()) return;

    DrawComponentHeader("Transform", true, [this, node]() {
        auto params = node->GetTransformParams();
        if (DrawColumnProperties("Transform", params.Unwrap(),
                                 &node->GetDefaultTransformParams())) {
            node->MarkDirty();
        }
    });
}

void PropertiesPanel::DrawHDRIBackdropProperties(EditorNode* node) {}

// Procedural mesh parameters. The panel stays generic: every generator registers its fields through
// PARAMS_LIST, so Cube, Sphere, Plane, Cylinder and Torus render here without extra code -- and a
// future generator does too, including its ParamControllerConfig (drag speed, minimum).
//
// MarkDirty is what makes an edit reach the GPU: the sync bridge only pushes dirty nodes and
// SceneObj::Update only rebuilds dirty mesh components. Mesh assets are cached by
// MeshParams::GetHash(), so a value change produces a new asset; once undo/redo lands, edits will
// be committed once per gesture instead of once per frame for that reason.
void PropertiesPanel::DrawMeshProperties(EditorNode* node) {
    if (!node) return;
    if (!node->HasMesh()) return;

    DrawComponentHeader("Mesh", true, [this, node]() {
        auto params = node->GetMeshParams();
        if (DrawColumnProperties("Mesh", params.As<MeshParamsObj>(),
                                 &node->GetDefaultMeshParams())) {
            node->MarkDirty();
        }
    });
}

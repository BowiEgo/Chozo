#pragma once

#include <Runtime/RenderCore/ParamsSnapshot.hpp>
#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include <cstddef>
#include <cstdint>

#include "../UIUtils.hpp"

#include <imgui.h>
#include <imgui_internal.h>

//=========================================================================================
// Property PropControllers
template <size_t I>
static void DrawAxis(const char* label, float* value, const ImVec2& buttonSize, ImFont* boldFont,
                     float valueSpeed, float resetValue, bool& valueChanged) {
    constexpr ImVec4 colors[3][3] = {
        { { 0.5f, 0.1f, 0.15f, 1.0f }, { 0.9f, 0.2f, 0.2f, 1.0f }, { 0.5f, 0.1f, 0.15f, 1.0f } },
        { { 0.2f, 0.5f, 0.2f, 1.0f }, { 0.3f, 0.8f, 0.3f, 1.0f }, { 0.2f, 0.5f, 0.2f, 1.0f } },
        { { 0.1f, 0.25f, 0.5f, 1.0f }, { 0.2f, 0.35f, 0.9f, 1.0f }, { 0.1f, 0.25f, 0.5f, 1.0f } }
    };

    {
        UIUtils::ScopedColorStack colorStack(ImGuiCol_Button, colors[I][0], ImGuiCol_ButtonHovered,
                                             colors[I][1], ImGuiCol_ButtonActive, colors[I][2]);
        UIUtils::ScopedFont fontScope(boldFont);

        UIUtils::ScopedID id(label);

        // The coloured label is a reset button for its axis. It is a real Button with the axis
        // colour applied through the style stack: hand-painting it with the draw list looked
        // identical in theory and rendered nothing in practice, so the proven approach stays.
        // The axis letter sits inside the coloured button. Drawing it as separate text (as this
        // used to) left the glyph sticking out to the left of an empty 8px square, which is what
        // read as "the label's text overflows to the left".
        if (ImGui::Button(label, buttonSize)) {
            *value       = resetValue;
            valueChanged = true;
        }
    }

    ImGui::SameLine();

    char dragLabel[4] = { '#', '#', label[0], '\0' };
    valueChanged |= ImGui::DragFloat(dragLabel, value, valueSpeed, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    if constexpr (I < 2) {
        ImGui::SameLine();
    }
}

static bool DrawVec3Control(const std::string& label, Vector3& values,
                            const float resetValue = 0.0f, const float valueSpeed = 0.1f,
                            const float columnWidth = 100.0f) {
    bool valueChanged = false;

    const ImGuiIO& io   = ImGui::GetIO();
    const auto boldFont = io.Fonts->Fonts[0];

    UIUtils::ScopedID id(label.c_str());

    UIUtils::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    const float lineHeight  = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    // A square as wide as a line; an 8px box is narrower than its own glyph, which pushed
    // the letter right instead of centring it.
    const ImVec2 buttonSize = { lineHeight, lineHeight };
    // The three fields and their three axis buttons share the controller width, so the fields get
    // what is left after the buttons; asking for more pushed Z outside the cell and it was clipped
    // away entirely, which is worse than losing its decimals.
    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() - buttonSize.x * 3.0f);

    DrawAxis<0>("X", &values.x, buttonSize, boldFont, valueSpeed, resetValue, valueChanged);
    DrawAxis<1>("Y", &values.y, buttonSize, boldFont, valueSpeed, resetValue, valueChanged);
    DrawAxis<2>("Z", &values.z, buttonSize, boldFont, valueSpeed, resetValue, valueChanged);

    return valueChanged;
}

static bool DrawTextureControl(const std::string& id, Texture texture) {
    UIUtils::ScopedID scopedID(id.c_str());

    UIUtils::DrawButtonImageByRatio(texture, { 120.0f, 120.0f });

    ImGui::SameLine();
    ImGui::BeginGroup();

    ImGuiComboFlags flags = 0;
    const char* items[]   = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE",    "FFFF", "GGGG",
                              "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
    int item_selected_idx = 0;
    const char* combo_preview_value = items[item_selected_idx];
    if (ImGui::BeginCombo("##Combo", combo_preview_value, flags)) {
        ImGuiTextFilter filter;
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
            filter.Clear();
        }
        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
        filter.Draw("##Filter", -FLT_MIN);

        for (int n = 0; n < IM_COUNTOF(items); n++) {
            const bool is_selected = (item_selected_idx == n);
            if (filter.PassFilter(items[n]))
                if (ImGui::Selectable(items[n], is_selected)) item_selected_idx = n;
        }
        ImGui::EndCombo();
    }

    if (UIUtils::IconButton("Inspect", IM_COL32(70, 160, 0, 255), IM_COL32(70, 180, 40, 255),
                            IM_COL32(100, 190, 40, 255))) {
    }
    ImGui::SameLine();
    if (UIUtils::IconButton("Remove", IM_COL32(210, 0, 0, 255), IM_COL32(210, 0, 40, 255),
                            IM_COL32(240, 16, 40, 255))) {
    }

    ImGui::EndGroup();

    return false;
}

template <typename T> bool DrawSlider(T& value, const std::string& name, float min, float max) {
    const std::string id = "##" + name;

    if constexpr (std::is_same_v<T, float>) {
        return ImGui::SliderFloat(id.c_str(), &value, min, max);
    } else if constexpr (std::is_same_v<T, double>) {
        float temp   = static_cast<float>(value);
        bool changed = ImGui::SliderFloat(id.c_str(), &temp, min, max);
        if (changed) {
            value = static_cast<double>(temp);
        }
        return changed;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return ImGui::SliderInt(id.c_str(), &value, static_cast<int>(min), static_cast<int>(max));
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        int temp = static_cast<int>(value);
        bool changed =
            ImGui::SliderInt(id.c_str(), &temp, static_cast<int>(min), static_cast<int>(max));
        if (changed) {
            value = static_cast<uint32_t>(temp);
            return true;
        }
        return false;
    }

    return false;
}

template <typename T>
bool DrawDrag(T& value, const std::string& name, float speed, float min, float max) {
    const std::string id = "##" + name;

    if constexpr (std::is_same_v<T, float>) {
        if (max > min) {
            return ImGui::DragFloat(id.c_str(), &value, speed, min, max);
        }
        return ImGui::DragFloat(id.c_str(), &value, speed);
    } else if constexpr (std::is_same_v<T, double>) {
        float temp   = static_cast<float>(value);
        bool changed = ImGui::DragFloat(id.c_str(), &temp, static_cast<float>(speed));
        if (changed) {
            value = static_cast<double>(temp);
        }
        return changed;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        if (max > min) {
            return ImGui::DragInt(id.c_str(), &value, static_cast<int>(speed),
                                  static_cast<int>(min), static_cast<int>(max));
        }
        return ImGui::DragInt(id.c_str(), &value, static_cast<int>(speed));
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        int temp     = static_cast<int>(value);
        int minInt   = static_cast<int>(min);
        int maxInt   = static_cast<int>(max);
        bool changed = false;

        if (maxInt > minInt) {
            changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed), minInt, maxInt);
        } else {
            changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed));
        }

        if (changed && temp >= 0) {
            value = static_cast<uint32_t>(temp);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, int64_t>) {
        int temp     = static_cast<int>(value);
        bool changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed));
        if (changed) {
            value = static_cast<int64_t>(temp);
        }
        return changed;
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        int temp     = static_cast<int>(value);
        bool changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed));
        if (changed && temp >= 0) {
            value = static_cast<uint64_t>(temp);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, Vector2>) {
        return ImGui::DragFloat2(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Vector3>) {
        // The coloured axis labels come from our own control, which is what the reference branch
        // calls here: ImGui's DragFloat3 has no per-axis affordance, and pointing this at
        // DrawVec3Control is what makes those labels exist at all.
        // Scale resets to 1 and everything else to 0, matching the semantic defaults Unity and
        // Unreal use and the field-level revert button.
        const float axisReset = (name.find("Scale") != std::string::npos) ? 1.0f : 0.0f;
        return DrawVec3Control(name, value, axisReset, speed);
    } else if constexpr (std::is_same_v<T, Vector4>) {
        return ImGui::DragFloat4(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Vector2>) {
        return ImGui::DragFloat2(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Vector3>) {
        // The coloured axis labels come from our own control, which is what the reference branch
        // calls here: ImGui's DragFloat3 has no per-axis affordance, and pointing this at
        // DrawVec3Control is what makes those labels exist at all.
        // Scale resets to 1 and everything else to 0, matching the semantic defaults Unity and
        // Unreal use and the field-level revert button.
        const float axisReset = (name.find("Scale") != std::string::npos) ? 1.0f : 0.0f;
        return DrawVec3Control(name, value, axisReset, speed);
    } else if constexpr (std::is_same_v<T, Vector4>) {
        return ImGui::DragFloat4(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Quaternion>) {
        Vector3 euler = value.ToEuler();
        bool changed  = DrawVec3Control(name, euler, 0.0f, speed);
        if (changed) {
            value = Quaternion::FromEuler(euler);
        }
        return changed;
    }

    return false;
}

template <typename T> bool DrawColor(T& value, const std::string& name) {
    const std::string id = "##" + name;

    if constexpr (std::is_same_v<T, Vector3>) {
        return ImGui::ColorEdit3(id.c_str(), &value.x);
    } else if constexpr (std::is_same_v<T, Vector4>) {
        return ImGui::ColorEdit4(id.c_str(), &value.x);
    } else if constexpr (std::is_same_v<T, Vector3>) {
        return ImGui::ColorEdit3(id.c_str(), &value.x);
    } else if constexpr (std::is_same_v<T, Vector4>) {
        return ImGui::ColorEdit4(id.c_str(), &value.x);
    }

    return false;
}

template <typename T>
bool DrawCombo(T& value, const std::string& name, const std::vector<std::string>& items,
               bool bNotifyChanged = true) {
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
        int current = static_cast<int>(value);
        if (ImGui::Combo(
                ("##" + name).c_str(), &current,
                [](void* data, int idx) {
                    return (*static_cast<const std::vector<std::string>*>(data))[idx].c_str();
                },
                (void*)&items, (int)items.size())) {
            value = current;
            return bNotifyChanged;
        }
    }
    return false;
}

template <typename T>
bool DrawDefaultController(T& value, const std::string& name, float speed = 0.01f, float min = 0.0f,
                           float max = 0.0f) {
    const std::string id = "##" + name;

    if constexpr (std::is_same_v<T, float>) {
        if (max > min) {
            return ImGui::DragFloat(id.c_str(), &value, speed, min, max);
        }
        return ImGui::DragFloat(id.c_str(), &value, speed);
    } else if constexpr (std::is_same_v<T, double>) {
        float temp   = static_cast<float>(value);
        bool changed = ImGui::DragFloat(id.c_str(), &temp, static_cast<float>(speed));
        if (changed) {
            value = static_cast<double>(temp);
        }
        return changed;
    } else if constexpr (std::is_same_v<T, int32_t>) {
        if (max > min) {
            return ImGui::DragInt(id.c_str(), &value, static_cast<int>(speed),
                                  static_cast<int>(min), static_cast<int>(max));
        }
        return ImGui::DragInt(id.c_str(), &value, static_cast<int>(speed));
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        int temp     = static_cast<int>(value);
        int minInt   = static_cast<int>(min);
        int maxInt   = static_cast<int>(max);
        bool changed = false;

        if (maxInt > minInt) {
            changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed), minInt, maxInt);
        } else {
            changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed));
        }

        if (changed && temp >= 0) {
            value = static_cast<uint32_t>(temp);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, int64_t>) {
        int temp     = static_cast<int>(value);
        bool changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed));
        if (changed) {
            value = static_cast<int64_t>(temp);
        }
        return changed;
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        int temp     = static_cast<int>(value);
        bool changed = ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed));
        if (changed && temp >= 0) {
            value = static_cast<uint64_t>(temp);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, bool>) {
        return ImGui::Checkbox(id.c_str(), &value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        char buffer[256];
        strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        if (ImGui::InputText(id.c_str(), buffer, sizeof(buffer))) {
            value = buffer;
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, Vector2>) {
        return ImGui::DragFloat2(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Vector3>) {
        // The coloured axis labels come from our own control, which is what the reference branch
        // calls here: ImGui's DragFloat3 has no per-axis affordance, and pointing this at
        // DrawVec3Control is what makes those labels exist at all.
        // Scale resets to 1 and everything else to 0, matching the semantic defaults Unity and
        // Unreal use and the field-level revert button.
        const float axisReset = (name.find("Scale") != std::string::npos) ? 1.0f : 0.0f;
        return DrawVec3Control(name, value, axisReset, speed);
    } else if constexpr (std::is_same_v<T, Vector4>) {
        return ImGui::DragFloat4(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Vector2>) {
        return ImGui::DragFloat2(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Vector4>) {
        return ImGui::DragFloat4(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, Quaternion>) {
        Vector3 euler = value.ToEuler();
        bool changed  = DrawVec3Control(name, euler, 0.0f, speed);
        if (changed) {
            value = Quaternion::FromEuler(euler);
        }
        return changed;
    } else if constexpr (std::is_enum_v<T>) {
        int temp = static_cast<int>(value);
        if (ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed))) {
            value = static_cast<T>(temp);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, AssetHandle>) {
        char buffer[64];
        return false;
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for DrawController");
        return false;
    }
}

template <typename T>
bool DrawControllerWithType(T& value, const std::string& name, const ParamControllerConfig config) {
    switch (config.Type) {
        case ParamControllerType::Slider: return DrawSlider(value, name, config.Min, config.Max);
        case ParamControllerType::Drag:
            return DrawDrag(value, name, config.Speed, config.Min, config.Max);
        case ParamControllerType::ColorPicker: return DrawColor(value, name);
        case ParamControllerType::Combo:
            return DrawCombo(value, name, config.Items, config.bNotifyDirty);
        case ParamControllerType::Default:
            return DrawDefaultController(value, name, config.Speed, config.Min, config.Max);
        default: return DrawDefaultController(value, name, config.Speed, config.Min, config.Max);
    }

    return DrawDefaultController(value, name, config.Speed, config.Min, config.Max);
}

class EditorParamsVisitor : public ParamsVisitor {
public:
    EditorParamsVisitor() = default;

    void SetReadOnly(bool readOnly) { m_ReadOnly = readOnly; }

    /// Reference values for the revert arrow: the parameter type's defaults.
    ///
    /// Defaults -- not the last saved state -- are what a per-field revert means in Unity
    /// ("Reset"), Unreal (the yellow reset arrow) and Blender ("Reset to Default Value"), and in
    /// all three the action is an ordinary undoable edit.
    void SetDefaults(const ParamsSnapshot* defaults) {
        m_Defaults = defaults;
        m_Cursor   = 0;
    }

    bool IsEditCommitted() const { return m_bEditCommitted; }
    void ResetEditCommitted() { m_bEditCommitted = false; }

    bool ConsumeResetRequest() {
        const bool requested = m_bResetRequested;
        m_bResetRequested    = false;
        return requested;
    }
    const std::string& ResetLabel() const { return m_ResetLabel; }

    /// One field: the controller, plus a revert arrow while the value differs from the default.
    ///
    /// The revert button is drawn after the controller, so the controller is told to leave room for
    /// it first. Without that reservation a full-width controller fills the table cell and pushes
    /// the button past the cell edge, where the table clips it -- the button exists but is
    /// invisible, which reads as "the feature is missing". AddTableRow's callback reports whether
    /// the value changed, so the widget's result is forwarded and a revert press adds to it.
    template <typename T>
    void DrawField(T& value, const std::string& name, const ParamControllerConfig& config) {
        const size_t index = m_Cursor++;

        AddTableRow(name, [&]() -> bool {
            // Decide up front whether the button will be drawn, so the controller can reserve
            // space.
            const auto* defaultValue = (m_Defaults && index < m_Defaults->Values.size())
                                           ? std::any_cast<T>(&m_Defaults->Values[index])
                                           : nullptr;
            const bool bShowRevert   = defaultValue && *defaultValue != value;

            if (bShowRevert) {
                const float buttonWidth = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x;
                ImGui::SetNextItemWidth(
                    std::max(1.0f, ImGui::GetContentRegionAvail().x - buttonWidth));
            }

            const bool bChanged = DrawControllerWithType(value, name, config);

            // Asked before the button is drawn: the button would otherwise become the "last item".
            if (ImGui::IsItemDeactivated()) {
                m_bEditCommitted = true;
            }

            bool bReverted = false;
            if (bShowRevert) {
                ImGui::SameLine();
                // Scope the button to its field, matching the convention used elsewhere in this
                // file: identical labels would otherwise share one ImGui ID, so hovering or
                // clicking one would apply to whichever instance ImGui resolved first.
                UIUtils::ScopedID scopedID(name.c_str());
                if (name == "Transform") {
                    // Vector fields revert per axis via the coloured labels; no per-row button.
                } else if (ImGui::SmallButton("R")) {
                    value             = *defaultValue;
                    bReverted         = true;
                    m_bResetRequested = true;
                    m_ResetLabel      = name;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Reset to default");
                }
            }

            return bChanged || bReverted;
        });
    }
    bool IsReadOnly() const { return m_ReadOnly; }
    bool IsValueChanged() const { return m_bValueChanged; }
    void ResetChangedFlag() { m_bValueChanged = false; }

    virtual void Visit(float& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(double& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(int32_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(uint32_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(int64_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(uint64_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(bool& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(std::string& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(Vector2& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(Vector3& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(Vector4& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(Quaternion& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

    virtual void Visit(AssetHandle& value, const std::string& name,
                       const ParamControllerConfig config = {}) override {
        DrawField(value, name, config);
    }

private:
    const ParamsSnapshot* m_Defaults = nullptr;
    size_t m_Cursor                  = 0;
    bool m_bEditCommitted            = false;
    bool m_bResetRequested           = false;
    std::string m_ResetLabel;
    template <typename DrawFunc> void AddTableRow(const std::string& name, DrawFunc&& drawFunc) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", name.c_str());

        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

        bool changed = drawFunc();
        if (changed) {
            m_bValueChanged = true;
        }

        ImGui::PopItemWidth();
    }

    bool m_ReadOnly      = false;
    bool m_bValueChanged = false;
};

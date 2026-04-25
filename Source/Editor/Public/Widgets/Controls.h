#pragma once

#include "AssetManager.h"
#include "CoreMinimal.h"
#include "EditorEvent.h"
#include "ImGuiLayer.h"
#include "Params.h"
#include "Ref.h"
#include "StringUtils.h"
#include "Texture.h"
#include "UIUtils.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <imgui.h>
#include <imgui_internal.h>

DECLARE_LOG_CATEGORY_EXTERN(LogPropertyControls, Info);

//=========================================================================================
// Property Controls
namespace ChozoEditor::Controls {

template <size_t I>
static void DrawAxis(const char* label, float* value, const ImVec2& buttonSize, ImFont* boldFont,
                     float valueSpeed, float resetValue, bool& valueChanged) {
    constexpr ImVec4 colors[3][3] = {
        { { 0.5f, 0.1f, 0.15f, 1.0f }, { 0.9f, 0.2f, 0.2f, 1.0f }, { 0.5f, 0.1f, 0.15f, 1.0f } },
        { { 0.2f, 0.5f, 0.2f, 1.0f }, { 0.3f, 0.8f, 0.3f, 1.0f }, { 0.2f, 0.5f, 0.2f, 1.0f } },
        { { 0.1f, 0.25f, 0.5f, 1.0f }, { 0.2f, 0.35f, 0.9f, 1.0f }, { 0.1f, 0.25f, 0.5f, 1.0f } }
    };

    {
        ChozoUtils::UI::ScopedColorStack colorStack(ImGuiCol_Button, colors[I][0],
                                                    ImGuiCol_ButtonHovered, colors[I][1],
                                                    ImGuiCol_ButtonActive, colors[I][2]);
        ChozoUtils::UI::ScopedFont fontScope(boldFont);

        ChozoUtils::UI::ScopedID id(label);

        ImGui::Text("%s", label);
        ImGui::SameLine();

        if (ImGui::Button("", buttonSize)) {
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

static bool DrawVec3Control(const std::string& label, FVector3& values,
                            const float resetValue = 0.0f, const float valueSpeed = 0.1f,
                            const float columnWidth = 100.0f) {
    bool valueChanged = false;

    const ImGuiIO& io   = ImGui::GetIO();
    const auto boldFont = io.Fonts->Fonts[0];

    ChozoUtils::UI::ScopedID id(label.c_str());

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ChozoUtils::UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    const float lineHeight  = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    const ImVec2 buttonSize = { 8.0f, lineHeight };

    DrawAxis<0>("X", &values.x, buttonSize, boldFont, valueSpeed, resetValue, valueChanged);
    DrawAxis<1>("Y", &values.y, buttonSize, boldFont, valueSpeed, resetValue, valueChanged);
    DrawAxis<2>("Z", &values.z, buttonSize, boldFont, valueSpeed, resetValue, valueChanged);

    return valueChanged;
}

static bool DrawTextureControl(TRef<CTexture> texture) {
    ChozoUtils::UI::DrawButtonImageByRatio(texture, { 120.0f, 120.0f });

    ImGui::SameLine();
    ImGui::BeginGroup();

    static ImGuiComboFlags flags = 0;
    const char* items[]          = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE",    "FFFF", "GGGG",
                                     "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
    static int item_selected_idx = 0;
    const char* combo_preview_value = items[item_selected_idx];
    if (ImGui::BeginCombo("combo 2 (w/ filter)", combo_preview_value, flags)) {
        static ImGuiTextFilter filter;
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

    if (ChozoUtils::UI::IconButton("Inspect", IM_COL32(70, 160, 0, 255), IM_COL32(70, 180, 40, 255),
                                   IM_COL32(100, 190, 40, 255))) {
    }
    ImGui::SameLine();
    if (ChozoUtils::UI::IconButton("Remove", IM_COL32(210, 0, 0, 255), IM_COL32(210, 0, 40, 255),
                                   IM_COL32(240, 16, 40, 255))) {
    }

    ImGui::EndGroup();

    return false;
}

static bool DrawMaterialControl(const TRef<CMaterial>& mat) {
    auto texture = CAssetManager::Get().GetCheckboardTexture();
    ChozoUtils::UI::DrawButtonImageByRatio(texture, { 120.0f, 120.0f });

    ImGui::SameLine();
    ImGui::BeginGroup();

    static ImGuiComboFlags flags = 0;
    const char* items[]          = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE",    "FFFF", "GGGG",
                                     "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
    static int item_selected_idx = 0;
    const char* combo_preview_value = items[item_selected_idx];
    if (ImGui::BeginCombo("combo 2 (w/ filter)", combo_preview_value, flags)) {
        static ImGuiTextFilter filter;
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

    if (ChozoUtils::UI::IconButton("Inspect", IM_COL32(70, 160, 0, 255), IM_COL32(70, 180, 40, 255),
                                   IM_COL32(100, 190, 40, 255))) {
        FOpenMaterialPanelEvent event(mat->GetHandle());
        FEventBus::Get().Dispatch(event);
    }
    ImGui::SameLine();
    if (ChozoUtils::UI::IconButton("Remove", IM_COL32(210, 0, 0, 255), IM_COL32(210, 0, 40, 255),
                                   IM_COL32(240, 16, 40, 255))) {
    }

    ImGui::EndGroup();

    return false;
}

template <typename T>
bool DrawControl(T& value, const std::string& name, float speed = 0.01f, float min = 0.0f,
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
    } else if constexpr (std::is_same_v<T, FVector2>) {
        return ImGui::DragFloat2(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, FVector3>) {
        return ImGui::DragFloat3(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, FVector4>) {
        return ImGui::DragFloat4(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, FVector2>) {
        return ImGui::DragFloat2(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, FVector3>) {
        return DrawVec3Control(name, value, 0.0f, speed);
        return ImGui::DragFloat3(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, FVector4>) {
        return ImGui::DragFloat4(id.c_str(), &value.x, speed);
    } else if constexpr (std::is_same_v<T, FQuaternion>) {
        FVector3 euler = value.ToEuler();
        bool changed   = ImGui::DragFloat3(id.c_str(), &euler.x, speed);
        if (changed) {
            value = FQuaternion::FromEuler(euler);
        }
        return changed;
    } else if constexpr (std::is_enum_v<T>) {
        int temp = static_cast<int>(value);
        if (ImGui::DragInt(id.c_str(), &temp, static_cast<int>(speed))) {
            value = static_cast<T>(temp);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<T, FAssetHandle>) {
        char buffer[64];
        TRef<IAsset> asset = CAssetManager::Get().GetAsset(value);
        if (asset) {
            if (asset->GetType() == EAssetType::Texture) {
                return DrawTextureControl(asset.As<CTexture>());
            }
            if (asset->GetType() == EAssetType::Material) {
                return DrawMaterialControl(asset);
            }
        }
        // if (asset) {
        //     std::string assetName = asset->GetName();
        //     strncpy(buffer, assetName.c_str(), sizeof(buffer) - 1);
        //     buffer[sizeof(buffer) - 1] = '\0';
        // } else {
        //     buffer[0] = '\0';
        // }
        // if (ImGui::InputText(id.c_str(), buffer, sizeof(buffer))) {
        //     // value = std::strtoull(buffer, nullptr, 10);
        //     return true;
        // }

        return false;
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type for DrawControl");
        return false;
    }
}

template <typename T>
bool DrawControlWithRange(T& value, const std::string& name, float speed = 0.01f, float min = 0.0f,
                          float max = 0.0f) {
    return DrawControl(value, name, speed, min, max);
}

template <typename T> bool DrawSlider(T& value, const std::string& name, float min, float max) {
    const std::string id = "##" + name;

    if constexpr (std::is_same_v<T, float>) {
        return ImGui::SliderFloat(id.c_str(), &value, min, max);
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

    return DrawControl(value, name);
}

template <typename T> bool DrawColor(T& value, const std::string& name) {
    const std::string id = "##" + name;

    if constexpr (std::is_same_v<T, FVector3>) {
        return ImGui::ColorEdit3(id.c_str(), &value.x);
    } else if constexpr (std::is_same_v<T, FVector4>) {
        return ImGui::ColorEdit4(id.c_str(), &value.x);
    } else if constexpr (std::is_same_v<T, FVector3>) {
        return ImGui::ColorEdit3(id.c_str(), &value.x);
    } else if constexpr (std::is_same_v<T, FVector4>) {
        return ImGui::ColorEdit4(id.c_str(), &value.x);
    }

    return DrawControl(value, name);
}

class TableParamsVisitor : public IParamsVisitor {
public:
    TableParamsVisitor() = default;

    void SetReadOnly(bool readOnly) { m_ReadOnly = readOnly; }
    bool IsReadOnly() const { return m_ReadOnly; }
    bool IsValueChanged() const { return m_bValueChanged; }
    void ResetChangedFlag() { m_bValueChanged = false; }

    virtual void Visit(float& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 0.01f); });
    }

    virtual void Visit(double& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 0.01f); });
    }

    virtual void Visit(int32_t& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 1.0f); });
    }

    virtual void Visit(uint32_t& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 1.0f, 0.0f, 1000.0f); });
    }

    virtual void Visit(int64_t& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 1.0f); });
    }

    virtual void Visit(uint64_t& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 1.0f, 0.0f, 1000.0f); });
    }

    virtual void Visit(bool& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name); });
    }

    virtual void Visit(std::string& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name); });
    }

    virtual void Visit(FVector2& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 0.01f); });
    }

    virtual void Visit(FVector3& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 0.01f); });
    }

    virtual void Visit(FVector4& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 0.01f); });
    }

    virtual void Visit(FQuaternion& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name, 1.0f); });
    }

    virtual void Visit(FAssetHandle& value, const std::string& name) override {
        AddTableRow(name, [&]() { return DrawControl(value, name); });
    }

private:
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

} // namespace ChozoEditor::Controls
#pragma once

#include "CoreMinimal.h"
#include "Params.h"
#include "Ref.h"
#include "StringUtils.h"
#include "Texture.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <imgui.h>
#include <imgui_internal.h>

DECLARE_LOG_CATEGORY_EXTERN(LogUIUtils, Info);

namespace ChozoUtils::UI {

#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_BLACK   0xFF000000
#define COLOR_RED     0xFF0000FF
#define COLOR_GREEN   0xFF00FF00
#define COLOR_BLUE    0xFFFF0000
#define COLOR_YELLOW  0xFF00FFFF
#define COLOR_CYAN    0xFFFFFF00
#define COLOR_MAGENTA 0xFFFF00FF

enum class ImGuiStyleType {
    Alpha,
    DisabledAlpha,
    WindowPadding,
    WindowRounding,
    WindowBorderSize,
    WindowMinSize,
    WindowTitleAlign,
    WindowMenuButtonPosition,
    ChildRounding,
    ChildBorderSize,
    PopupRounding,
    PopupBorderSize,
    FramePadding,
    FrameRounding,
    FrameBorderSize,
    ItemSpacing,
    ItemInnerSpacing,
    CellPadding,
    TouchExtraPadding,
    IndentSpacing,
    ColumnsMinSpacing,
    ScrollbarSize,
    ScrollbarRounding,
    GrabMinSize,
    GrabRounding,
    LogSliderDeadzone,
    TabRounding,
    TabBorderSize,
    TabCloseButtonMinWidthUnselected,
    TabBarBorderSize,
    TableAngledHeadersAngle,
    TableAngledHeadersTextAlign,
    ColorButtonPosition,
    ButtonTextAlign,
    SelectableTextAlign,
    SeparatorTextBorderSize,
    SeparatorTextAlign,
    SeparatorTextPadding,
    DisplayWindowPadding,
    DisplaySafeAreaPadding,
    DockingSeparatorSize,
    MouseCursorScale,
    AntiAliasedLines,
    AntiAliasedLinesUseTex,
    AntiAliasedFill,
    CurveTessellationTol,
    CircleTessellationMaxError,
    HoverStationaryDelay,
    HoverDelayShort,
    HoverDelayNormal,
    HoverFlagsForTooltipNav
};

enum class ImGuiFontStyle {
    FallbackAdvanceX,
    FontSize,
    Scale,
    Ascent,
    Descent,
};

struct ImGuiFontExtended : public ImFont {
    ImFontBaked* GetBaked() { return LastBaked; }

    template <typename T> T& GetStyleVar(ImGuiFontStyle fontStyle) {
        if constexpr (std::is_same_v<T, float>) {
            switch (fontStyle) {
                case ImGuiFontStyle::FallbackAdvanceX: return GetBaked()->FallbackAdvanceX;
                case ImGuiFontStyle::FontSize: return GetBaked()->Size;
                case ImGuiFontStyle::Scale: return Scale;
                case ImGuiFontStyle::Ascent: return GetBaked()->Ascent;
                case ImGuiFontStyle::Descent: return GetBaked()->Descent;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static T dummy = T();
                    return dummy;
            }
        }
    }
};

struct ImGuiStyleExtended : public ImGuiStyle {
    template <typename T> T& GetStyleVar(ImGuiStyleType styleVar) {
        if constexpr (std::is_same_v<T, float>) {
            switch (styleVar) {
                case ImGuiStyleType::Alpha: return Alpha;
                case ImGuiStyleType::DisabledAlpha: return DisabledAlpha;
                case ImGuiStyleType::WindowRounding: return WindowRounding;
                case ImGuiStyleType::WindowBorderSize: return WindowBorderSize;
                case ImGuiStyleType::ChildRounding: return ChildRounding;
                case ImGuiStyleType::ChildBorderSize: return ChildBorderSize;
                case ImGuiStyleType::PopupRounding: return PopupRounding;
                case ImGuiStyleType::PopupBorderSize: return PopupBorderSize;
                case ImGuiStyleType::FrameRounding: return FrameRounding;
                case ImGuiStyleType::FrameBorderSize: return FrameBorderSize;
                case ImGuiStyleType::IndentSpacing: return IndentSpacing;
                case ImGuiStyleType::ColumnsMinSpacing: return ColumnsMinSpacing;
                case ImGuiStyleType::ScrollbarSize: return ScrollbarSize;
                case ImGuiStyleType::ScrollbarRounding: return ScrollbarRounding;
                case ImGuiStyleType::GrabMinSize: return GrabMinSize;
                case ImGuiStyleType::GrabRounding: return GrabRounding;
                case ImGuiStyleType::LogSliderDeadzone: return LogSliderDeadzone;
                case ImGuiStyleType::TabRounding: return TabRounding;
                case ImGuiStyleType::TabBorderSize: return TabBorderSize;
                case ImGuiStyleType::TabCloseButtonMinWidthUnselected:
                    return TabCloseButtonMinWidthUnselected;
                case ImGuiStyleType::TabBarBorderSize: return TabBarBorderSize;
                case ImGuiStyleType::CurveTessellationTol: return CurveTessellationTol;
                case ImGuiStyleType::CircleTessellationMaxError: return CircleTessellationMaxError;
                case ImGuiStyleType::HoverStationaryDelay: return HoverStationaryDelay;
                case ImGuiStyleType::HoverDelayShort: return HoverDelayShort;
                case ImGuiStyleType::HoverDelayNormal: return HoverDelayNormal;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static float dummy = 0.0f;
                    return dummy;
            }
        } else if constexpr (std::is_same_v<T, ImGuiHoveredFlags>) {

            switch (styleVar) {
                case ImGuiStyleType::HoverFlagsForTooltipNav: return HoverFlagsForTooltipNav;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static int dummy = 0;
                    return dummy;
            }
        } else if constexpr (std::is_same_v<T, ImVec2>) {
            switch (styleVar) {
                case ImGuiStyleType::WindowPadding: return WindowPadding;
                case ImGuiStyleType::FramePadding: return FramePadding;
                case ImGuiStyleType::ItemSpacing: return ItemSpacing;
                case ImGuiStyleType::ItemInnerSpacing: return ItemInnerSpacing;
                case ImGuiStyleType::CellPadding: return CellPadding;
                case ImGuiStyleType::TouchExtraPadding: return TouchExtraPadding;
                case ImGuiStyleType::DisplayWindowPadding: return DisplayWindowPadding;
                case ImGuiStyleType::DisplaySafeAreaPadding: return DisplaySafeAreaPadding;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static ImVec2 dummy(0.0f, 0.0f);
                    return dummy;
            }
        } else if constexpr (std::is_same_v<T, ImVec4>) {
            switch (styleVar) {
                case ImGuiStyleType::SeparatorTextPadding: return SeparatorTextPadding;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static ImVec4 dummy(0.0f, 0.0f, 0.0f, 0.0f);
                    return dummy;
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            switch (styleVar) {
                case ImGuiStyleType::AntiAliasedLines: return AntiAliasedLines;
                case ImGuiStyleType::AntiAliasedLinesUseTex: return AntiAliasedLinesUseTex;
                case ImGuiStyleType::AntiAliasedFill: return AntiAliasedFill;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static bool dummy = false;
                    return dummy;
            }
        } else if constexpr (std::is_same_v<T, ImGuiDir>) {
            switch (styleVar) {
                case ImGuiStyleType::WindowMenuButtonPosition: return WindowMenuButtonPosition;
                default:
                    IM_ASSERT(false && "Unsupported style variable!");
                    static ImGuiDir dummy = ImGuiDir_None;
                    return dummy;
            }
        }
    }
};

class ScopedStyle {
public:
    ScopedStyle(const ScopedStyle&)            = delete;
    ScopedStyle& operator=(const ScopedStyle&) = delete;
    template <typename T> ScopedStyle(ImGuiStyleVar styleVar, T value) {
        ImGui::PushStyleVar(styleVar, value);
    }
    ~ScopedStyle() { ImGui::PopStyleVar(); }
};

template <typename T> class ScopedFrameStyle {
public:
    ScopedFrameStyle(const ScopedFrameStyle&)            = delete;
    ScopedFrameStyle& operator=(const ScopedFrameStyle&) = delete;
    ScopedFrameStyle(ImGuiStyleType styleType, T value) {
        ImGuiStyleExtended& style       = static_cast<ImGuiStyleExtended&>(ImGui::GetStyle());
        m_StyleType                     = styleType;
        m_OldValue                      = style.GetStyleVar<T>(styleType);
        style.GetStyleVar<T>(styleType) = value;
    }
    ~ScopedFrameStyle() {
        ImGuiStyleExtended& style         = static_cast<ImGuiStyleExtended&>(ImGui::GetStyle());
        style.GetStyleVar<T>(m_StyleType) = m_OldValue;
    }

private:
    ImGuiStyleType m_StyleType;
    T m_OldValue;
};

class ScopedColor {
public:
    ScopedColor(const ScopedColor&)            = delete;
    ScopedColor& operator=(const ScopedColor&) = delete;
    template <typename T> ScopedColor(ImGuiCol colourId, T colour) {
        ImGui::PushStyleColor(colourId, ImColor(colour).Value);
    }
    ~ScopedColor() { ImGui::PopStyleColor(); }
};

class ScopedFont {
public:
    ScopedFont(const ScopedFont&)            = delete;
    ScopedFont& operator=(const ScopedFont&) = delete;
    ScopedFont(ImFont* font) { ImGui::PushFont(font); }
    ~ScopedFont() { ImGui::PopFont(); }
};

template <typename T> class ScopedFontStyle {
public:
    ScopedFontStyle(const ScopedFontStyle&)            = delete;
    ScopedFontStyle& operator=(const ScopedFontStyle&) = delete;
    ScopedFontStyle(ImGuiFontStyle fontStyle, T value) {
        ImGuiFontExtended* font         = static_cast<ImGuiFontExtended*>(ImGui::GetFont());
        m_FontStyle                     = fontStyle;
        m_OldValue                      = font->GetStyleVar<T>(fontStyle);
        font->GetStyleVar<T>(fontStyle) = value;
        ImGui::PushFont(font);
    }
    ~ScopedFontStyle() {
        ImGuiFontExtended* font           = static_cast<ImGuiFontExtended*>(ImGui::GetFont());
        font->GetStyleVar<T>(m_FontStyle) = m_OldValue;
        ImGui::PopFont();
    }

private:
    ImGuiFontStyle m_FontStyle;
    T m_OldValue;
};

class ScopedID {
public:
    ScopedID(const ScopedID&)            = delete;
    ScopedID& operator=(const ScopedID&) = delete;
    template <typename T> ScopedID(T id) { ImGui::PushID(id); }
    ~ScopedID() { ImGui::PopID(); }
};

class ScopedColorStack {
public:
    ScopedColorStack(const ScopedColorStack&)            = delete;
    ScopedColorStack& operator=(const ScopedColorStack&) = delete;
    template <typename ColorType, typename... OtherColors>
    ScopedColorStack(ImGuiCol firstColorID, ColorType firstColor, OtherColors&&... otherColorPairs)
        : m_Count((sizeof...(otherColorPairs) / 2) + 1) {
        static_assert((sizeof...(otherColorPairs) & 1u) == 0,
                      "ScopedColorStack constructor expects a list of pairs of color IDs and "
                      "colors as its arguments");

        PushColor(firstColorID, firstColor, std::forward<OtherColors>(otherColorPairs)...);
    }

    ~ScopedColorStack() { ImGui::PopStyleColor(m_Count); }

private:
    int m_Count;

    template <typename ColorType, typename... OtherColors>
    void PushColor(ImGuiCol colorID, ColorType color, OtherColors&&... otherColorPairs) {
        if constexpr (sizeof...(otherColorPairs) == 0) {
            ImGui::PushStyleColor(colorID, ImColor(color).Value);
        } else {
            ImGui::PushStyleColor(colorID, ImColor(color).Value);
            PushColor(std::forward<OtherColors>(otherColorPairs)...);
        }
    }
};

inline ImColor ColourWithValue(const ImColor& color, float value) {
    const ImVec4& colRaw = color.Value;
    float hue, sat, val;
    ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
    return ImColor::HSV(hue, sat, std::min(value, 1.0f));
}

inline ImColor ColourWithSaturation(const ImColor& color, float saturation) {
    const ImVec4& colRaw = color.Value;
    float hue, sat, val;
    ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
    return ImColor::HSV(hue, std::min(saturation, 1.0f), val);
}

inline ImColor ColourWithHue(const ImColor& color, float hue) {
    const ImVec4& colRaw = color.Value;
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, h, s, v);
    return ImColor::HSV(std::min(hue, 1.0f), s, v);
}

inline ImColor ColourWithAlpha(const ImColor& color, float multiplier) {
    ImVec4 colRaw = color.Value;
    colRaw.w      = multiplier;
    return colRaw;
}

inline ImColor ColourWithMultipliedValue(const ImColor& color, float multiplier) {
    const ImVec4& colRaw = color.Value;
    float hue, sat, val;
    ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
    return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
}

inline ImColor ColourWithMultipliedSaturation(const ImColor& color, float multiplier) {
    const ImVec4& colRaw = color.Value;
    float hue, sat, val;
    ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
    return ImColor::HSV(hue, std::min(sat * multiplier, 1.0f), val);
}

inline ImColor ColourWithMultipliedHue(const ImColor& color, float multiplier) {
    const ImVec4& colRaw = color.Value;
    float hue, sat, val;
    ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
    return ImColor::HSV(std::min(hue * multiplier, 1.0f), sat, val);
}

inline ImColor ColourWithMultipliedAlpha(const ImColor& color, float multiplier) {
    ImVec4 colRaw = color.Value;
    colRaw.w *= multiplier;
    return colRaw;
}

//=========================================================================================
// Cursor
static void ShiftCursorX(float distance) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
}

static void ShiftCursorY(float distance) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
}

static void ShiftCursor(float x, float y) {
    const ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + x, cursor.y + y));
}

//=========================================================================================
// Rectangle
inline ImRect GetItemRect() { return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()); }

inline ImRect RectExpanded(const ImRect& rect, float x, float y) {
    ImRect result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

inline ImRect RectOffset(const ImRect& rect, float x, float y) {
    ImRect result = rect;
    result.Min.x += x;
    result.Min.y += y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

inline ImRect RectOffset(const ImRect& rect, ImVec2 xy) { return RectOffset(rect, xy.x, xy.y); }

//=========================================================================================
// Button Image
inline void DrawButtonImage(TRef<CTexture>& image, ImU32 tint, ImVec2 rectMin, ImVec2 rectMax,
                            ImVec2 uv0, ImVec2 uv1) {
    auto* drawList = ImGui::GetWindowDrawList();
    drawList->AddImage((ImTextureID)image->GetDescriptorSet(), rectMin, rectMax, uv0, uv1, tint);
};

inline void DrawButtonImage(TRef<CTexture>& image, ImU32 tint, ImRect rectangle, ImVec2 uv0,
                            ImVec2 uv1) {
    DrawButtonImage(image, tint, rectangle.Min, rectangle.Max, uv0, uv1);
};

inline void DrawButtonImage(TRef<CTexture>& imageNormal, TRef<CTexture>& imageHovered,
                            TRef<CTexture>& imagePressed, ImU32 tintNormal, ImU32 tintHovered,
                            ImU32 tintPressed, ImVec2 rectMin, ImVec2 rectMax, ImVec2 uv0,
                            ImVec2 uv1) {
    auto* drawList = ImGui::GetWindowDrawList();
    if (ImGui::IsItemActive())
        drawList->AddImage((ImTextureID)imagePressed->GetDescriptorSet(), rectMin, rectMax, uv0,
                           uv1, tintPressed);
    else if (ImGui::IsItemHovered())
        drawList->AddImage((ImTextureID)imageHovered->GetDescriptorSet(), rectMin, rectMax, uv0,
                           uv1, tintHovered);
    else
        drawList->AddImage((ImTextureID)imageNormal->GetDescriptorSet(), rectMin, rectMax, uv0, uv1,
                           tintNormal);
};

inline void DrawButtonImage(TRef<CTexture>& imageNormal, TRef<CTexture>& imageHovered,
                            TRef<CTexture>& imagePressed, ImU32 tintNormal, ImU32 tintHovered,
                            ImU32 tintPressed, ImRect rectangle, ImVec2 uv0 = { 0, 0 },
                            ImVec2 uv1 = { 1, 1 }) {
    DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed,
                    rectangle.Min, rectangle.Max, uv0, uv1);
};

inline void DrawButtonImage(TRef<CTexture>& image, ImU32 tintNormal, ImU32 tintHovered,
                            ImU32 tintPressed, ImVec2 rectMin, ImVec2 rectMax,
                            ImVec2 uv0 = { 0, 0 }, ImVec2 uv1 = { 1, 1 }) {
    DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax,
                    uv0, uv1);
};

inline void DrawButtonImage(TRef<CTexture>& image, ImU32 tintNormal, ImU32 tintHovered,
                            ImU32 tintPressed, ImRect rectangle, ImVec2 uv0 = { 0, 0 },
                            ImVec2 uv1 = { 1, 1 }) {
    DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min,
                    rectangle.Max, uv0, uv1);
};

inline void DrawButtonImage(TRef<CTexture>& imageNormal, TRef<CTexture>& imageHovered,
                            TRef<CTexture>& imagePressed, ImU32 tintNormal, ImU32 tintHovered,
                            ImU32 tintPressed, ImVec2 uv0 = { 0, 0 }, ImVec2 uv1 = { 1, 1 }) {
    DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed,
                    ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), uv0, uv1);
};

inline void DrawButtonImage(TRef<CTexture>& image, ImU32 tintNormal, ImU32 tintHovered,
                            ImU32 tintPressed) {
    DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed,
                    ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
};

inline void DrawButtonImageByRatio(TRef<CTexture>& image) {
    float imageAspectRatio =
        static_cast<float>(image->GetSize().Height) / static_cast<float>(image->GetSize().Width);
    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);
    if (imageAspectRatio <= 1.0f) {
        float offsetY = (1.0f - 1.0f / imageAspectRatio) / 2.0f;
        uv0.y         = 1.0f - offsetY;
        uv1.y         = offsetY;
    } else {
        float offsetX = (1.0f - imageAspectRatio) / 2.0f;
        uv0.x         = offsetX;
        uv1.x         = 1.0f - offsetX;
    }

    UI::DrawButtonImage(image, IM_COL32(255, 255, 255, 225),
                        UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f), uv0, uv1);
}

//=========================================================================================
// IconButton
static bool IconButton(std::string label, ImU32 bgNormal, ImU32 bgHovered, ImU32 bgPressed) {
    UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 0.0f));
    UI::ScopedFrameStyle<float> frameRounding(ImGuiStyleType::FrameRounding, 2.0f);
    UI::ScopedFontStyle<float> font(ImGuiFontStyle::Scale, 1.2f);
    UI::ScopedColorStack buttonColors(ImGuiCol_Button, bgNormal, ImGuiCol_ButtonHovered, bgHovered,
                                      ImGuiCol_ButtonActive, bgPressed);

    return ImGui::Button(label.c_str());
}

//=========================================================================================
// FileButton
// static void FileButton(std::string* filePath) {
//     ImGui::Button("...", ImVec2(40.0f, 0.0f));
//     if (ImGui::BeginDragDropTarget()) {
//         if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
//             const wchar_t* path = (const wchar_t*)payload->Data;
//             fs::path fullPath = Utils::File::GetAssetDirectory() / fs::path((char*)path);

//             CZ_INFO("FileButton: {0}", fullPath.string().c_str());
//             *filePath = fullPath.string();
//         }

//         ImGui::EndDragDropTarget();
//     }
// }
//=========================================================================================
// DragAndDrop
// static void BeginDragAndDrop(std::function<void(AssetHandle handle)>&& func) {
//     if (ImGui::BeginDragDropTarget()) {
//         if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
//             const wchar_t* handle_wchar = (const wchar_t*)payload->Data;
//             AssetHandle handle = Utils::WChar::WCharToUint64(handle_wchar);
//             CZ_INFO("Drop target: {0}", std::to_string(handle));

//             func(handle);
//         }

//         ImGui::EndDragDropTarget();
//     }
// }

static void DrawDashedRect(ImVec2 min, ImVec2 max, ImU32 color, float thickness = 1.0f,
                           float dashLength = 5.0f, float gapLength = 5.0f) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 size;
    size.x = max.x - min.x;
    size.y = max.y - min.y;

    // Top side
    for (float x = min.x; x < max.x; x += dashLength + gapLength) {
        float endX = ImMin(x + dashLength, max.x);
        drawList->AddLine(ImVec2(x, min.y), ImVec2(endX, min.y), color, thickness);
    }

    // Bottom side
    for (float x = min.x; x < max.x; x += dashLength + gapLength) {
        float endX = ImMin(x + dashLength, max.x);
        drawList->AddLine(ImVec2(x, max.y), ImVec2(endX, max.y), color, thickness);
    }

    // Left side
    for (float y = min.y; y < max.y; y += dashLength + gapLength) {
        float endY = ImMin(y + dashLength, max.y);
        drawList->AddLine(ImVec2(min.x, y), ImVec2(min.x, endY), color, thickness);
    }

    // Right side
    for (float y = min.y; y < max.y; y += dashLength + gapLength) {
        float endY = ImMin(y + dashLength, max.y);
        drawList->AddLine(ImVec2(max.x, y), ImVec2(max.x, endY), color, thickness);
    }
}

TRef<CTexture> LoadSVGIcon(const std::string& name, int targetSize = 32,
                           uint32 strokeColor = COLOR_WHITE);

unsigned char* LoadImagePreview(const char* path, int max_size, int* out_w, int* out_h);

std::vector<std::string> GetWrappedFileName(const char* label, float WrapWidth, float MaxLineHeight,
                                            float RowSpacing, ImVec2 RawTextSize);

//=========================================================================================
// Property Control
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
        if (ImGui::InputText(id.c_str(), buffer, sizeof(buffer))) {
            // value = std::strtoull(buffer, nullptr, 10);
            return true;
        }
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

} // namespace ChozoUtils::UI
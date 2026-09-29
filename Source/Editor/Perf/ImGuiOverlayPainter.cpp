#include "ImGuiOverlayPainter.hpp"

#include <string>
#include <vector>

#include <imgui.h>

namespace CZ {
namespace {
constexpr float kPadding  = 8.0f;
constexpr float kLineGap  = 2.0f;
constexpr float kPlotH    = 36.0f;
constexpr float kRounding = 4.0f;

inline ImU32 ToImU32(const OverlayColor& c) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(c.R, c.G, c.B, c.A));
}

std::string ToStd(std::string_view v) { return std::string(v); }
} // namespace

void ImGuiOverlayPainter::BeginPanel(std::string_view, float x, float y, float width) {
    // Channel 1 is the background layer: it is drawn last but composited first.
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->ChannelsSplit(2);
    list->ChannelsSetCurrent(1);

    m_X       = x;
    m_Y       = y;
    m_Width   = width;
    m_CursorY = y + kPadding;
    m_Height  = kPadding * 2.0f;
    m_Active  = true;
}

void ImGuiOverlayPainter::Text(std::string_view text, const OverlayColor& color) {
    if (!m_Active) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->ChannelsSetCurrent(0);

    const std::string owned = ToStd(text);
    list->AddText(ImVec2(m_X + kPadding, m_CursorY), ToImU32(color), owned.c_str());

    const float line = ImGui::GetTextLineHeight();
    m_CursorY += line + kLineGap;
    m_Height += line + kLineGap;
}

void ImGuiOverlayPainter::Separator() {
    if (!m_Active) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->ChannelsSetCurrent(0);

    const float y = m_CursorY + kLineGap;
    list->AddLine(ImVec2(m_X + kPadding, y), ImVec2(m_X + m_Width - kPadding, y),
                  ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
    m_CursorY += kLineGap * 2.0f + 2.0f;
    m_Height += kLineGap * 2.0f + 2.0f;
}

void ImGuiOverlayPainter::Plot(std::string_view, const float* values, size_t count, float minValue,
                               float maxValue, const OverlayColor& color) {
    if (!m_Active || !values || count < 2) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->ChannelsSetCurrent(0);

    const float x0     = m_X + kPadding;
    const float innerW = m_Width - kPadding * 2.0f;
    const float range  = (maxValue - minValue) != 0.0f ? (maxValue - minValue) : 1.0f;

    std::vector<ImVec2> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count - 1);
        float v       = (values[i] - minValue) / range;
        v             = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
        points.emplace_back(x0 + innerW * t, m_CursorY + kPlotH * (1.0f - v));
    }

    list->AddRectFilled(ImVec2(x0, m_CursorY), ImVec2(x0 + innerW, m_CursorY + kPlotH),
                        ImGui::GetColorU32(ImGuiCol_FrameBg, 0.55f), kRounding);
    list->AddPolyline(points.data(), static_cast<int>(points.size()), ToImU32(color), 0, 1.5f);

    m_CursorY += kPlotH + kLineGap;
    m_Height += kPlotH + kLineGap;
}

void ImGuiOverlayPainter::EndPanel() {
    if (!m_Active) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->ChannelsSetCurrent(1);
    list->AddRectFilled(ImVec2(m_X, m_Y), ImVec2(m_X + m_Width, m_Y + m_Height),
                        ImGui::GetColorU32(ImGuiCol_WindowBg, 0.55f), kRounding);
    list->ChannelsMerge();
    m_Active = false;
}

} // namespace CZ

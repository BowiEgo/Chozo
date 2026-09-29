#include "ImGuiOverlayPainter.hpp"

#include <imgui.h>

namespace CZ {

void ImGuiOverlayPainter::BeginPanel(std::string_view id, float x, float y, float width) {
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    constexpr ImGuiWindowFlags kFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin(id.data(), nullptr, kFlags);
}

void ImGuiOverlayPainter::EndPanel() { ImGui::End(); }

void ImGuiOverlayPainter::Text(std::string_view text, const OverlayColor& color) {
    ImGui::TextColored(ImVec4(color.R, color.G, color.B, color.A), "%.*s",
                       static_cast<int>(text.size()), text.data());
}

void ImGuiOverlayPainter::Separator() { ImGui::Separator(); }

void ImGuiOverlayPainter::Plot(std::string_view label, const float* values, size_t count,
                               float minValue, float maxValue, const OverlayColor& color) {
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(color.R, color.G, color.B, color.A));
    ImGui::PlotLines(label.data(), values, static_cast<int>(count), 0, nullptr, minValue, maxValue,
                     ImVec2(ImGui::GetContentRegionAvail().x, 36.0f));
    ImGui::PopStyleColor();
}

} // namespace CZ

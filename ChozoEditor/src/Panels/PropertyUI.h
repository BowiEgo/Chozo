#pragma once

#include "Chozo.h"

#include <imgui_internal.h>

namespace Chozo {

    static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float valueSpeed = 0.1f, float columnWidth = 100.0f)
    {
        bool valueChanged = false;

        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { 8.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.5f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("", buttonSize))
        {
            values.x = resetValue;
            valueChanged = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##X", &values.x, valueSpeed, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.5f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.5f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("", buttonSize))
        {
            values.y = resetValue;
            valueChanged = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Y", &values.y, valueSpeed, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.5f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.5f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("", buttonSize))
        {
            values.z = resetValue;
            valueChanged = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        valueChanged |= ImGui::DragFloat("##Z", &values.z, valueSpeed, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PopStyleVar();

        // ImGui::Columns(1);

        ImGui::PopID();

        return valueChanged;
    }

    template<typename targetValueType, typename UIFunction>
    static void DrawColumnValue(const std::string& name, targetValueType& target, UIFunction uiFunction)
    {
        const ImGuiTableFlags flags = ImGuiTableFlags_Resizable;
        if (ImGui::BeginTable("table", 2, flags))
        {
            ImGui::SetNextItemWidth(50.0f);
            ImGui::TableNextRow();
            for (int column = 0; column < 2; column++)
            {
                ImGui::TableSetColumnIndex(column);
                if (column == 0)
                {
                    ImGui::Text("%s", name.c_str());
                    ImGui::TableNextColumn();
                }
                else
                {
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                    uiFunction(target);
                    ImGui::PopItemWidth();
                }
            }
            ImGui::EndTable();
        }
    }
}
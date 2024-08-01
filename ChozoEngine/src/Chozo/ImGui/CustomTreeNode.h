#pragma once

#include "Chozo/Renderer/Texture.h"

#include <imgui.h>

namespace ImGui {
	bool TreeNodeWithIcon(Chozo::Ref<Chozo::Texture2D> icon, ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, ImColor iconTint = IM_COL32_WHITE);
	bool TreeNodeWithIcon(Chozo::Ref<Chozo::Texture2D> icon, const void* ptr_id, ImGuiTreeNodeFlags flags, ImColor iconTint, const char* fmt, ...);
	bool TreeNodeWithIcon(Chozo::Ref<Chozo::Texture2D> icon, const char* label, ImGuiTreeNodeFlags flags, ImColor iconTint = IM_COL32_WHITE);
}

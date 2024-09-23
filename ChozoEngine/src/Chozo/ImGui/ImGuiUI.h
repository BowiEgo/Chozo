#pragma once

#include "Chozo/Renderer/Texture.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <vector>

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;
}

namespace Chozo::UI {

    class ScopedStyle
    {
    public:
        ScopedStyle(const ScopedStyle&) = delete;
        ScopedStyle& operator=(const ScopedStyle&) = delete;
        template <typename T>
        ScopedStyle(ImGuiStyleVar styleVar, T value) { ImGui::PushStyleVar(styleVar, value); }
        ~ScopedStyle() { ImGui::PopStyleVar(); }
    };

    class ScopedColorStack
    {
    public:
        ScopedColorStack(const ScopedColorStack&) = delete;
        ScopedColorStack& operator=(const ScopedColorStack&) = delete;
        template <typename ColorType, typename... OtherColors>
        ScopedColorStack(ImGuiCol firstColorID, ColorType firstColor, OtherColors&& ... otherColorPairs)
            : m_Count((sizeof... (otherColorPairs) / 2) + 1)
        {
            static_assert ((sizeof... (otherColorPairs) & 1u) == 0,
                "ScopedColorStack constructor expects a list of pairs of color IDs and colors as its arguments");
            
            PushColor(firstColorID, firstColor, std::forward<OtherColors>(otherColorPairs)...);
        }

        ~ScopedColorStack() { ImGui::PopStyleColor(m_Count); }
    private:
        int m_Count;

        template <typename ColorType, typename... OtherColors>
        void PushColor(ImGuiCol colorID, ColorType color, OtherColors&& ... otherColorPairs)
        {
            if constexpr (sizeof... (otherColorPairs) == 0)
            {
                ImGui::PushStyleColor(colorID, ImColor(color).Value);
            }
            else
            {
                ImGui::PushStyleColor(colorID, ImColor(color).Value);
                PushColor(std::forward<OtherColors>(otherColorPairs)...);
            }
        }
    };

    inline ImColor ColourWithValue(const ImColor& color, float value)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(value, 1.0f));
	}

	inline ImColor ColourWithSaturation(const ImColor& color, float saturation)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, std::min(saturation, 1.0f), val);
	}

	inline ImColor ColourWithHue(const ImColor& color, float hue)
	{
		const ImVec4& colRaw = color.Value;
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, h, s, v);
		return ImColor::HSV(std::min(hue, 1.0f), s, v);
	}

	inline ImColor ColourWithAlpha(const ImColor& color, float multiplier)
	{
		ImVec4 colRaw = color.Value;
		colRaw.w = multiplier;
		return colRaw;
	}

	inline ImColor ColourWithMultipliedValue(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
	}

	inline ImColor ColourWithMultipliedSaturation(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(hue, std::min(sat * multiplier, 1.0f), val);
	}

	inline ImColor ColourWithMultipliedHue(const ImColor& color, float multiplier)
	{
		const ImVec4& colRaw = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
		return ImColor::HSV(std::min(hue * multiplier, 1.0f), sat, val);
	}

	inline ImColor ColourWithMultipliedAlpha(const ImColor& color, float multiplier)
	{
		ImVec4 colRaw = color.Value;
		colRaw.w *= multiplier;
		return colRaw;
	}

	//=========================================================================================
	/// Cursor

	static void ShiftCursorX(float distance)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
	}

	static void ShiftCursorY(float distance)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
	}

	static void ShiftCursor(float x, float y)
	{
		const ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cursor.x + x, cursor.y + y));
	}

    //=========================================================================================
	/// Rectangle

	inline ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	inline ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	inline ImRect RectOffset(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x += x;
		result.Min.y += y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	inline ImRect RectOffset(const ImRect& rect, ImVec2 xy)
	{
		return RectOffset(rect, xy.x, xy.y);
	}

    //=========================================================================================
	/// Button Image

	inline void DrawButtonImage(const Ref<Texture2D>& image, ImU32 tint, ImVec2 rectMin, ImVec2 rectMax, ImVec2 uv0, ImVec2 uv1)
	{
		auto* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage((ImTextureID)(uintptr_t)image->GetRendererID(), rectMin, rectMax, uv0, uv1, tint);
	};

    inline void DrawButtonImage(const Ref<Texture2D>& image, ImU32 tint, ImRect rectangle, ImVec2 uv0, ImVec2 uv1)
	{
		DrawButtonImage(image, tint, rectangle.Min, rectangle.Max, uv0, uv1);
	};

	inline void DrawButtonImage(const Ref<Texture2D>& imageNormal, const Ref<Texture2D>& imageHovered, const Ref<Texture2D>& imagePressed,
		 ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		 ImVec2 rectMin, ImVec2 rectMax, ImVec2 uv0, ImVec2 uv1)
	{
		auto* drawList = ImGui::GetWindowDrawList();
		if (ImGui::IsItemActive())
			drawList->AddImage((ImTextureID)(uintptr_t)imagePressed->GetRendererID(), rectMin, rectMax, uv0, uv1, tintPressed);
		else if (ImGui::IsItemHovered())
			drawList->AddImage((ImTextureID)(uintptr_t)imageHovered->GetRendererID(), rectMin, rectMax, uv0, uv1, tintHovered);
		else
			drawList->AddImage((ImTextureID)(uintptr_t)imageNormal->GetRendererID(), rectMin, rectMax, uv0, uv1, tintNormal);
	};

	inline void DrawButtonImage(const Ref<Texture2D>& imageNormal, const Ref<Texture2D>& imageHovered, const Ref<Texture2D>& imagePressed,
		 ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		 ImRect rectangle, ImVec2 uv0 = {0, 0}, ImVec2 uv1 = {1, 1})
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max, uv0, uv1);
	};

	inline void DrawButtonImage(const Ref<Texture2D>& image,
		 ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		 ImVec2 rectMin, ImVec2 rectMax, ImVec2 uv0 = {0, 0}, ImVec2 uv1 = {1, 1})
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax, uv0, uv1);
	};

	inline void DrawButtonImage(const Ref<Texture2D>& image,
		 ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		 ImRect rectangle, ImVec2 uv0 = {0, 0}, ImVec2 uv1 = {1, 1})
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max, uv0, uv1);
	};


	inline void DrawButtonImage(const Ref<Texture2D>& imageNormal, const Ref<Texture2D>& imageHovered, const Ref<Texture2D>& imagePressed,
		 ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed, ImVec2 uv0 = {0, 0}, ImVec2 uv1 = {1, 1})
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), uv0, uv1);
	};

	inline void DrawButtonImage(const Ref<Texture2D>& image,
		 ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	//=========================================================================================
	/// FileButton

	static void FileButton(std::string* filePath)
	{
		ImGui::Button("...", ImVec2(40.0f, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path fullPath = g_AssetsPath / std::filesystem::path((char*)path);

            	CZ_INFO("FileButton: {0}", fullPath.string().c_str());
				*filePath = fullPath.string();
			}

			ImGui::EndDragDropTarget();
		}
	}
}
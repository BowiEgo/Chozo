#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <vector>

#include "Chozo/Renderer/Texture.h"
#include "Chozo/Utilities/StringUtils.h"

namespace Chozo {

	enum class ImGuiStyleType
	{
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
		TabMinWidthForCloseButton,
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

	enum class ImGuiFontStyle
	{
		FallbackAdvanceX,
		FontSize,
		EllipsisWidth,
		EllipsisCharStep,
		Scale,
		Ascent, Descent,
	};

	struct ImGuiFontExtended : public ImFont
	{
		template<typename T>
		T& GetStyleVar(ImGuiFontStyle fontStyle)
		{
			if constexpr (std::is_same_v<T, float>)
			{
				switch (fontStyle)
				{
					case ImGuiFontStyle::FallbackAdvanceX:	return FallbackAdvanceX;
					case ImGuiFontStyle::FontSize:			return FontSize;
					case ImGuiFontStyle::EllipsisWidth:		return EllipsisWidth;
					case ImGuiFontStyle::EllipsisCharStep:	return EllipsisCharStep;
					case ImGuiFontStyle::Scale:				return Scale;
					case ImGuiFontStyle::Ascent:			return Ascent;
					case ImGuiFontStyle::Descent:			return Descent;
					default:
						IM_ASSERT(false && "Unsupported style variable!");
						static float dummy = 0.0f;
						return dummy;
				}
			}
		}
	};

	struct ImGuiStyleExtended : public ImGuiStyle
	{
		template<typename T>
		T& GetStyleVar(ImGuiStyleType styleVar)
		{
			if constexpr (std::is_same_v<T, float>)
			{
				switch (styleVar)
				{
					case ImGuiStyleType::Alpha:            return Alpha;
					case ImGuiStyleType::DisabledAlpha:    return DisabledAlpha;
					case ImGuiStyleType::WindowRounding:   return WindowRounding;
					case ImGuiStyleType::WindowBorderSize: return WindowBorderSize;
					case ImGuiStyleType::ChildRounding:    return ChildRounding;
					case ImGuiStyleType::ChildBorderSize:  return ChildBorderSize;
					case ImGuiStyleType::PopupRounding:    return PopupRounding;
					case ImGuiStyleType::PopupBorderSize:  return PopupBorderSize;
					case ImGuiStyleType::FrameRounding:    return FrameRounding;
					case ImGuiStyleType::FrameBorderSize:  return FrameBorderSize;
					case ImGuiStyleType::IndentSpacing:    return IndentSpacing;
					case ImGuiStyleType::ColumnsMinSpacing:return ColumnsMinSpacing;
					case ImGuiStyleType::ScrollbarSize:    return ScrollbarSize;
					case ImGuiStyleType::ScrollbarRounding:return ScrollbarRounding;
					case ImGuiStyleType::GrabMinSize:      return GrabMinSize;
					case ImGuiStyleType::GrabRounding:     return GrabRounding;
					case ImGuiStyleType::LogSliderDeadzone:return LogSliderDeadzone;
					case ImGuiStyleType::TabRounding:      return TabRounding;
					case ImGuiStyleType::TabBorderSize:    return TabBorderSize;
					case ImGuiStyleType::TabMinWidthForCloseButton: return TabMinWidthForCloseButton;
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
			}
			else if constexpr (std::is_same_v<T, ImGuiHoveredFlags>)
			{

				switch (styleVar)
				{
					case ImGuiStyleType::HoverFlagsForTooltipNav: return HoverFlagsForTooltipNav;
				default:
						IM_ASSERT(false && "Unsupported style variable!");
						static int dummy = 0;
						return dummy;
				}
			}
			else if constexpr (std::is_same_v<T, ImVec2>)
			{
				switch (styleVar)
				{
					case ImGuiStyleType::WindowPadding:    return WindowPadding;
					case ImGuiStyleType::FramePadding:     return FramePadding;
					case ImGuiStyleType::ItemSpacing:      return ItemSpacing;
					case ImGuiStyleType::ItemInnerSpacing: return ItemInnerSpacing;
					case ImGuiStyleType::CellPadding:      return CellPadding;
					case ImGuiStyleType::TouchExtraPadding:return TouchExtraPadding;
					case ImGuiStyleType::DisplayWindowPadding: return DisplayWindowPadding;
					case ImGuiStyleType::DisplaySafeAreaPadding: return DisplaySafeAreaPadding;
					default:
						IM_ASSERT(false && "Unsupported style variable!");
						static ImVec2 dummy(0.0f, 0.0f);
						return dummy;
				}
			}
			else if constexpr (std::is_same_v<T, ImVec4>)
			{
				switch (styleVar)
				{
					case ImGuiStyleType::SeparatorTextPadding: return SeparatorTextPadding;
					default:
						IM_ASSERT(false && "Unsupported style variable!");
						static ImVec4 dummy(0.0f, 0.0f, 0.0f, 0.0f);
						return dummy;
				}
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				switch (styleVar)
				{
					case ImGuiStyleType::AntiAliasedLines:        return AntiAliasedLines;
					case ImGuiStyleType::AntiAliasedLinesUseTex:  return AntiAliasedLinesUseTex;
					case ImGuiStyleType::AntiAliasedFill:         return AntiAliasedFill;
					default:
						IM_ASSERT(false && "Unsupported style variable!");
						static bool dummy = false;
						return dummy;
				}
			}
			else if constexpr (std::is_same_v<T, ImGuiDir>)
			{
				switch (styleVar)
				{
					case ImGuiStyleType::WindowMenuButtonPosition: return WindowMenuButtonPosition;
					default:
						IM_ASSERT(false && "Unsupported style variable!");
						static ImGuiDir dummy = ImGuiDir_None;
						return dummy;
				}
			}
		}
	};
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

	template <typename T>
	class ScopedFrameStyle
    {
    public:
        ScopedFrameStyle(const ScopedFrameStyle&) = delete;
        ScopedFrameStyle& operator=(const ScopedFrameStyle&) = delete;
        ScopedFrameStyle(ImGuiStyleType styleType, T value) {
			ImGuiStyleExtended& style = static_cast<ImGuiStyleExtended&>(ImGui::GetStyle());
			m_StyleType = styleType;
			m_OldValue = style.GetStyleVar<T>(styleType);
			style.GetStyleVar<T>(styleType) = value;
		}
        ~ScopedFrameStyle() {
			ImGuiStyleExtended& style = static_cast<ImGuiStyleExtended&>(ImGui::GetStyle());
			style.GetStyleVar<T>(m_StyleType) = m_OldValue;
		}
	private:
        ImGuiStyleType m_StyleType;
		T m_OldValue;
    };

	class ScopedColor
	{
	public:
		ScopedColor(const ScopedColor&) = delete;
		ScopedColor& operator=(const ScopedColor&) = delete;
		template<typename T>
		ScopedColor(ImGuiCol colourId, T colour) { ImGui::PushStyleColor(colourId, ImColor(colour).Value); }
		~ScopedColor() { ImGui::PopStyleColor(); }
	};

	class ScopedFont
	{
	public:
		ScopedFont(const ScopedFont&) = delete;
		ScopedFont& operator=(const ScopedFont&) = delete;
		ScopedFont(ImFont* font) { ImGui::PushFont(font); }
		~ScopedFont() { ImGui::PopFont(); }
	};

	template <typename T>
	class ScopedFontStyle
	{
	public:
		ScopedFontStyle(const ScopedFontStyle&) = delete;
		ScopedFontStyle& operator=(const ScopedFontStyle&) = delete;
		ScopedFontStyle(ImGuiFontStyle fontStyle, T value) {
			ImGuiFontExtended* font = static_cast<ImGuiFontExtended*>(ImGui::GetFont());
			m_FontStyle = fontStyle;
			m_OldValue = font->GetStyleVar<T>(fontStyle);
			font->GetStyleVar<T>(fontStyle) = value;
			ImGui::PushFont(font);
		}
        ~ScopedFontStyle() {
			ImGuiFontExtended* font = static_cast<ImGuiFontExtended*>(ImGui::GetFont());
			font->GetStyleVar<T>(m_FontStyle) = m_OldValue;
			ImGui::PopFont();
		}
	private:
		ImGuiFontStyle m_FontStyle;
		T m_OldValue;
	};

	class ScopedID
	{
	public:
		ScopedID(const ScopedID&) = delete;
		ScopedID& operator=(const ScopedID&) = delete;
		template<typename T>
		ScopedID(T id) { ImGui::PushID(id); }
		~ScopedID() { ImGui::PopID(); }
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
	/// IconButton
	static bool IconButton(std::string label, ImU32 bgNormal, ImU32 bgHovered, ImU32 bgPressed)
	{
		UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 0.0f));
		UI::ScopedFrameStyle<float> frameRounding(ImGuiStyleType::FrameRounding, 2.0f);
		UI::ScopedFontStyle<float> font(ImGuiFontStyle::Scale, 1.2f);
		UI::ScopedColorStack buttonColors(ImGuiCol_Button, bgNormal,
										  ImGuiCol_ButtonHovered, bgHovered,
										  ImGuiCol_ButtonActive, bgPressed);

        return ImGui::Button(label.c_str());
	}

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
				fs::path fullPath = Utils::File::GetAssetDirectory() / fs::path((char*)path);

            	CZ_INFO("FileButton: {0}", fullPath.string().c_str());
				*filePath = fullPath.string();
			}

			ImGui::EndDragDropTarget();
		}
	}
	//=========================================================================================
	/// DragAndDrop
	static void BeginDragAndDrop(std::function<void(AssetHandle handle)>&& func)
	{
		if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* handle_wchar = (const wchar_t*)payload->Data;
                AssetHandle handle = Utils::WChar::WCharToUint64(handle_wchar);
                CZ_INFO("Drop target: {0}", std::to_string(handle));

				func(handle);
            }

            ImGui::EndDragDropTarget();
        }
	}

	static void DrawDashedRect(ImVec2 min, ImVec2 max, ImU32 color, float thickness = 1.0f, float dashLength = 5.0f, float gapLength = 5.0f)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 size;
		size.x = max.x - min.x;
		size.y = max.y - min.y;

		// Top side
		for (float x = min.x; x < max.x; x += dashLength + gapLength)
		{
			float endX = ImMin(x + dashLength, max.x);
			drawList->AddLine(ImVec2(x, min.y), ImVec2(endX, min.y), color, thickness);
		}

		// Bottom side
		for (float x = min.x; x < max.x; x += dashLength + gapLength)
		{
			float endX = ImMin(x + dashLength, max.x);
			drawList->AddLine(ImVec2(x, max.y), ImVec2(endX, max.y), color, thickness);
		}

		// Left side
		for (float y = min.y; y < max.y; y += dashLength + gapLength)
		{
			float endY = ImMin(y + dashLength, max.y);
			drawList->AddLine(ImVec2(min.x, y), ImVec2(min.x, endY), color, thickness);
		}

		// Right side
		for (float y = min.y; y < max.y; y += dashLength + gapLength)
		{
			float endY = ImMin(y + dashLength, max.y);
			drawList->AddLine(ImVec2(max.x, y), ImVec2(max.x, endY), color, thickness);
		}
	}
}
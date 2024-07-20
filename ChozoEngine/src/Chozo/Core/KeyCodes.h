#pragma once

namespace Chozo {

    typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

    typedef enum class MouseButton : uint16_t
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Left = Button0,
		Right = Button1,
		Middle = Button2
	} Button;

    inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, MouseButton button)
	{
		os << static_cast<int32_t>(button);
		return os;
	}
}

// From glfw3.h
#define CZ_KEY_SPACE           ::Chozo::Key::Space
#define CZ_KEY_APOSTROPHE      ::Chozo::Key::Apostrophe    /* ' */
#define CZ_KEY_COMMA           ::Chozo::Key::Comma         /* , */
#define CZ_KEY_MINUS           ::Chozo::Key::Minus         /* - */
#define CZ_KEY_PERIOD          ::Chozo::Key::Period        /* . */
#define CZ_KEY_SLASH           ::Chozo::Key::Slash         /* / */
#define CZ_KEY_0               ::Chozo::Key::D0
#define CZ_KEY_1               ::Chozo::Key::D1
#define CZ_KEY_2               ::Chozo::Key::D2
#define CZ_KEY_3               ::Chozo::Key::D3
#define CZ_KEY_4               ::Chozo::Key::D4
#define CZ_KEY_5               ::Chozo::Key::D5
#define CZ_KEY_6               ::Chozo::Key::D6
#define CZ_KEY_7               ::Chozo::Key::D7
#define CZ_KEY_8               ::Chozo::Key::D8
#define CZ_KEY_9               ::Chozo::Key::D9
#define CZ_KEY_SEMICOLON       ::Chozo::Key::Semicolon     /* ; */
#define CZ_KEY_EQUAL           ::Chozo::Key::Equal         /* = */
#define CZ_KEY_A               ::Chozo::Key::A
#define CZ_KEY_B               ::Chozo::Key::B
#define CZ_KEY_C               ::Chozo::Key::C
#define CZ_KEY_D               ::Chozo::Key::D
#define CZ_KEY_E               ::Chozo::Key::E
#define CZ_KEY_F               ::Chozo::Key::F
#define CZ_KEY_G               ::Chozo::Key::G
#define CZ_KEY_H               ::Chozo::Key::H
#define CZ_KEY_I               ::Chozo::Key::I
#define CZ_KEY_J               ::Chozo::Key::J
#define CZ_KEY_K               ::Chozo::Key::K
#define CZ_KEY_L               ::Chozo::Key::L
#define CZ_KEY_M               ::Chozo::Key::M
#define CZ_KEY_N               ::Chozo::Key::N
#define CZ_KEY_O               ::Chozo::Key::O
#define CZ_KEY_P               ::Chozo::Key::P
#define CZ_KEY_Q               ::Chozo::Key::Q
#define CZ_KEY_R               ::Chozo::Key::R
#define CZ_KEY_S               ::Chozo::Key::S
#define CZ_KEY_T               ::Chozo::Key::T
#define CZ_KEY_U               ::Chozo::Key::U
#define CZ_KEY_V               ::Chozo::Key::V
#define CZ_KEY_W               ::Chozo::Key::W
#define CZ_KEY_X               ::Chozo::Key::X
#define CZ_KEY_Y               ::Chozo::Key::Y
#define CZ_KEY_Z               ::Chozo::Key::Z
#define CZ_KEY_LEFT_BRACKET    ::Chozo::Key::LeftBracket   /* [ */
#define CZ_KEY_BACKSLASH       ::Chozo::Key::Backslash     /* \ */
#define CZ_KEY_RIGHT_BRACKET   ::Chozo::Key::RightBracket  /* ] */
#define CZ_KEY_GRAVE_ACCENT    ::Chozo::Key::GraveAccent   /* ` */
#define CZ_KEY_WORLD_1         ::Chozo::Key::World1        /* non-US #1 */
#define CZ_KEY_WORLD_2         ::Chozo::Key::World2        /* non-US #2 */

/* Function keys */
#define CZ_KEY_ESCAPE          ::Chozo::Key::Escape
#define CZ_KEY_ENTER           ::Chozo::Key::Enter
#define CZ_KEY_TAB             ::Chozo::Key::Tab
#define CZ_KEY_BACKSPACE       ::Chozo::Key::Backspace
#define CZ_KEY_INSERT          ::Chozo::Key::Insert
#define CZ_KEY_DELETE          ::Chozo::Key::Delete
#define CZ_KEY_RIGHT           ::Chozo::Key::Right
#define CZ_KEY_LEFT            ::Chozo::Key::Left
#define CZ_KEY_DOWN            ::Chozo::Key::Down
#define CZ_KEY_UP              ::Chozo::Key::Up
#define CZ_KEY_PAGE_UP         ::Chozo::Key::PageUp
#define CZ_KEY_PAGE_DOWN       ::Chozo::Key::PageDown
#define CZ_KEY_HOME            ::Chozo::Key::Home
#define CZ_KEY_END             ::Chozo::Key::End
#define CZ_KEY_CAPS_LOCK       ::Chozo::Key::CapsLock
#define CZ_KEY_SCROLL_LOCK     ::Chozo::Key::ScrollLock
#define CZ_KEY_NUM_LOCK        ::Chozo::Key::NumLock
#define CZ_KEY_PRINT_SCREEN    ::Chozo::Key::PrintScreen
#define CZ_KEY_PAUSE           ::Chozo::Key::Pause
#define CZ_KEY_F1              ::Chozo::Key::F1
#define CZ_KEY_F2              ::Chozo::Key::F2
#define CZ_KEY_F3              ::Chozo::Key::F3
#define CZ_KEY_F4              ::Chozo::Key::F4
#define CZ_KEY_F5              ::Chozo::Key::F5
#define CZ_KEY_F6              ::Chozo::Key::F6
#define CZ_KEY_F7              ::Chozo::Key::F7
#define CZ_KEY_F8              ::Chozo::Key::F8
#define CZ_KEY_F9              ::Chozo::Key::F9
#define CZ_KEY_F10             ::Chozo::Key::F10
#define CZ_KEY_F11             ::Chozo::Key::F11
#define CZ_KEY_F12             ::Chozo::Key::F12
#define CZ_KEY_F13             ::Chozo::Key::F13
#define CZ_KEY_F14             ::Chozo::Key::F14
#define CZ_KEY_F15             ::Chozo::Key::F15
#define CZ_KEY_F16             ::Chozo::Key::F16
#define CZ_KEY_F17             ::Chozo::Key::F17
#define CZ_KEY_F18             ::Chozo::Key::F18
#define CZ_KEY_F19             ::Chozo::Key::F19
#define CZ_KEY_F20             ::Chozo::Key::F20
#define CZ_KEY_F21             ::Chozo::Key::F21
#define CZ_KEY_F22             ::Chozo::Key::F22
#define CZ_KEY_F23             ::Chozo::Key::F23
#define CZ_KEY_F24             ::Chozo::Key::F24
#define CZ_KEY_F25             ::Chozo::Key::F25

/* Keypad */
#define CZ_KEY_KP_0            ::Chozo::Key::KP0
#define CZ_KEY_KP_1            ::Chozo::Key::KP1
#define CZ_KEY_KP_2            ::Chozo::Key::KP2
#define CZ_KEY_KP_3            ::Chozo::Key::KP3
#define CZ_KEY_KP_4            ::Chozo::Key::KP4
#define CZ_KEY_KP_5            ::Chozo::Key::KP5
#define CZ_KEY_KP_6            ::Chozo::Key::KP6
#define CZ_KEY_KP_7            ::Chozo::Key::KP7
#define CZ_KEY_KP_8            ::Chozo::Key::KP8
#define CZ_KEY_KP_9            ::Chozo::Key::KP9
#define CZ_KEY_KP_DECIMAL      ::Chozo::Key::KPDecimal
#define CZ_KEY_KP_DIVIDE       ::Chozo::Key::KPDivide
#define CZ_KEY_KP_MULTIPLY     ::Chozo::Key::KPMultiply
#define CZ_KEY_KP_SUBTRACT     ::Chozo::Key::KPSubtract
#define CZ_KEY_KP_ADD          ::Chozo::Key::KPAdd
#define CZ_KEY_KP_ENTER        ::Chozo::Key::KPEnter
#define CZ_KEY_KP_EQUAL        ::Chozo::Key::KPEqual

#define CZ_KEY_LEFT_SHIFT      ::Chozo::Key::LeftShift
#define CZ_KEY_LEFT_CONTROL    ::Chozo::Key::LeftControl
#define CZ_KEY_LEFT_ALT        ::Chozo::Key::LeftAlt
#define CZ_KEY_LEFT_SUPER      ::Chozo::Key::LeftSuper
#define CZ_KEY_RIGHT_SHIFT     ::Chozo::Key::RightShift
#define CZ_KEY_RIGHT_CONTROL   ::Chozo::Key::RightControl
#define CZ_KEY_RIGHT_ALT       ::Chozo::Key::RightAlt
#define CZ_KEY_RIGHT_SUPER     ::Chozo::Key::RightSuper
#define CZ_KEY_MENU            ::Chozo::Key::Menu

// Mouse
#define CZ_MOUSE_BUTTON_LEFT    ::Chozo::Button::Left
#define CZ_MOUSE_BUTTON_RIGHT   ::Chozo::Button::Right
#define CZ_MOUSE_BUTTON_MIDDLE  ::Chozo::Button::Middle

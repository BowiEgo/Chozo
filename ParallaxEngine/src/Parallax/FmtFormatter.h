#if FMT_VERSION >= 90000
    template <> struct fmt::formatter<Parallax::Event> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::WindowCloseEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::AppTickEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::AppUpdateEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::AppRenderEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::KeyPressedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::KeyReleasedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::MouseMovedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::MouseScrolledEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::MouseButtonPressedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Parallax::MouseButtonReleasedEvent> : ostream_formatter{};
#endif

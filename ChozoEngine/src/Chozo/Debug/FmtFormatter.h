#if FMT_VERSION >= 90000
    template <> struct fmt::formatter<Chozo::Event> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::WindowCloseEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::AppTickEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::AppUpdateEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::AppRenderEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::KeyPressedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::KeyReleasedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::MouseMovedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::MouseScrolledEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::MouseButtonPressedEvent> : ostream_formatter{};
    template <> struct fmt::formatter<Chozo::MouseButtonReleasedEvent> : ostream_formatter{};
#endif

#pragma once

#include <fmt/format.h>

#include <string>

namespace CZ {

/**
 * Component for storing a human-readable name for an Entity
 *
 * Usage:
 * - Editor hierarchy display
 * - Debug logging
 * - Asset references
 * - Search functionality
 */
struct NameComponent {
    std::string Name;

    NameComponent() = default;
    explicit NameComponent(const std::string& name) : Name(name) {}
    explicit NameComponent(const char* name) : Name(name) {}

    // Comparison operators
    bool operator==(const NameComponent& other) const { return Name == other.Name; }

    bool operator!=(const NameComponent& other) const { return Name != other.Name; }

    // Implicit conversion to string
    operator std::string() const { return Name; }
    operator const char*() const { return Name.c_str(); }

    // Check if empty
    bool IsEmpty() const { return Name.empty(); }

    // Get length
    size_t Length() const { return Name.length(); }

    // Clear name
    void Clear() { Name.clear(); }

    // Set name
    void SetName(const std::string& name) { Name = name; }
    void SetName(const char* name) { Name = name; }

    // Append to name
    NameComponent& operator+=(const std::string& suffix) {
        Name += suffix;
        return *this;
    }

    // Get mutable reference for editing
    std::string& GetMutable() { return Name; }
    const std::string& Get() const { return Name; }
};

// Stream output
inline std::ostream& operator<<(std::ostream& os, const NameComponent& name) {
    os << name.Name;
    return os;
}

} // namespace CZ

// fmt support for logging
namespace fmt {
template <> struct formatter<CZ::NameComponent> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CZ::NameComponent& name, FormatContext& ctx) const {
        return format_to(ctx.out(), "{}", name.Name);
    }
};
} // namespace fmt

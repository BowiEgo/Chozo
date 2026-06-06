#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <ostream>
#include <sstream>
#include <string>

namespace CZ {

class Entity {
public:
    static constexpr uint32_t Invalid = UINT32_MAX;

    Entity() : m_Handle(Invalid) {}
    explicit Entity(uint32_t handle) : m_Handle(handle) {}

    uint32_t GetHandle() const { return m_Handle; }
    bool IsValid() const { return m_Handle != Invalid; }

    explicit operator bool() const { return IsValid(); }
    operator uint32_t() const { return m_Handle; }

    bool operator==(const Entity& other) const { return m_Handle == other.m_Handle; }
    bool operator!=(const Entity& other) const { return !(*this == other); }

    std::string ToString() const {
        if (IsValid()) {
            return "Entity[" + std::to_string(m_Handle) + "]";
        } else {
            return "Entity[invalid]";
        }
    }

private:
    uint32_t m_Handle = Invalid;
};

inline std::ostream& operator<<(std::ostream& os, const Entity& entity) {
    os << entity.ToString();
    return os;
}

} // namespace CZ

namespace fmt {
template <> struct formatter<CZ::Entity> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const CZ::Entity& entity, FormatContext& ctx) const {
        std::stringstream ss;
        ss << entity;
        return formatter<std::string>::format(ss.str(), ctx);
    }
};
} // namespace fmt

namespace std {
template <> struct hash<CZ::Entity> {
    size_t operator()(const CZ::Entity& entity) const {
        return hash<uint32_t>()(entity.GetHandle());
    }
};
} // namespace std
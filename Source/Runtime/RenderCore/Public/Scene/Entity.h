#pragma once

#include <entt/entt.hpp>
#include <spdlog/fmt/bundled/format.h>

#include <ostream>
#include <sstream>

class FEntity {
public:
    FEntity() = default;
    FEntity(entt::entity handle) : m_Handle(handle) {}
    FEntity(uint32_t handle) : m_Handle(static_cast<entt::entity>(handle)) {}

    entt::entity GetHandle() const { return m_Handle; }
    bool IsValid() const { return m_Handle != entt::null; }

    operator bool() const { return m_Handle != entt::null; }
    operator entt::entity() const { return m_Handle; }
    operator uint32_t() const { return static_cast<uint32_t>(m_Handle); }

    bool operator==(const FEntity& other) const { return m_Handle == other.m_Handle; }
    bool operator!=(const FEntity& other) const { return !(*this == other); }

    std::string ToString() const {
        if (IsValid()) {
            return "Entity[" + std::to_string(static_cast<uint32_t>(m_Handle)) + "]";
        } else {
            return "Entity[invalid]";
        }
    }

private:
    entt::entity m_Handle = entt::null;
};

// Stream output
inline std::ostream& operator<<(std::ostream& os, const FEntity& entity) {
    os << entity.ToString();
    return os;
}

namespace fmt {
template <> struct formatter<FEntity> : formatter<std::string> {
    template <typename FormatContext> auto format(const FEntity& entity, FormatContext& ctx) const {
        std::stringstream ss;
        ss << entity;
        return formatter<std::string>::format(ss.str(), ctx);
    }
};
} // namespace fmt

// std::hash support
namespace std {
template <> struct hash<FEntity> {
    size_t operator()(const FEntity& entity) const {
        return hash<entt::entity>()(entity.GetHandle());
    }
};
} // namespace std
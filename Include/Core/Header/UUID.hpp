#pragma once

#include <Core/Header/Types.h>

#include <string>

namespace CZ {

struct UUID {
public:
    // ===== Constructors =====
    UUID();
    explicit UUID(uint64_t low, uint64_t high);
    explicit UUID(const std::string& str);
    ~UUID() = default;

    // ===== Copy & Move =====
    UUID(const UUID& other)                = default;
    UUID(UUID&& other) noexcept            = default;
    UUID& operator=(const UUID& other)     = default;
    UUID& operator=(UUID&& other) noexcept = default;

    // ===== Comparison =====
    bool operator==(const UUID& other) const;
    bool operator!=(const UUID& other) const;
    bool operator<(const UUID& other) const;

    // ===== Accessors =====
    uint64_t GetLow() const { return m_Low; }
    uint64_t GetHigh() const { return m_High; }

    // ===== Conversion =====
    std::string ToString() const;
    explicit operator std::string() const { return ToString(); }

    std::pair<uint64_t, uint64_t> ToPair() const { return { m_Low, m_High }; }

    // ===== Validation =====
    bool IsValid() const { return m_Low != 0 || m_High != 0; }
    static UUID Invalid() { return UUID(0, 0); }

    // ===== Static generators =====
    static UUID Generate();
    static UUID FromString(const std::string& str);

private:
    uint64_t m_Low;
    uint64_t m_High;
};

} // namespace CZ

// Hash support for unordered containers
namespace std {
template <> struct hash<CZ::UUID> {
    size_t operator()(const CZ::UUID& uuid) const {
        uint64_t combined = uuid.GetLow() ^ uuid.GetHigh();
        return hash<uint64_t>()(combined);
    }
};
} // namespace std

#pragma once

#include "CoreExport.h"
#include "CoreMinimal.h"

#include <algorithm>

class CORE_API UUID {
public:
    // ===== Constructors =====
    UUID();
    explicit UUID(uint64_t low, uint64_t high);
    explicit UUID(const std::string& str);
    ~UUID() = default;

    // ===== Copy & Move =====
    UUID(const UUID& other) = default;
    UUID(UUID&& other) noexcept = default;
    UUID& operator=(const UUID& other) = default;
    UUID& operator=(UUID&& other) noexcept = default;

    // ===== Comparison =====
    bool operator==(const UUID& other) const;
    bool operator!=(const UUID& other) const;
    bool operator<(const UUID& other) const;

    // ===== Conversion =====
    std::string ToString() const;
    explicit operator std::string() const { return ToString(); }
    explicit operator uint64_t() const { return m_Low; }

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

// Hash support for unordered containers
namespace std {
template <> struct hash<UUID> {
    size_t operator()(const UUID& uuid) const {
        return hash<uint64_t>()(static_cast<uint64_t>(uuid)) ^
               hash<uint64_t>()(static_cast<uint64_t>(uuid) >> 32);
    }
};
} // namespace std

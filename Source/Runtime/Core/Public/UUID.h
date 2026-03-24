#pragma once

#include "CoreExport.h"
#include "CoreMinimal.h"

#include <algorithm>

class CORE_API FUUID {
public:
    // ===== Constructors =====
    FUUID();
    explicit FUUID(uint64_t low, uint64_t high);
    explicit FUUID(const std::string& str);
    ~FUUID() = default;

    // ===== Copy & Move =====
    FUUID(const FUUID& other) = default;
    FUUID(FUUID&& other) noexcept = default;
    FUUID& operator=(const FUUID& other) = default;
    FUUID& operator=(FUUID&& other) noexcept = default;

    // ===== Comparison =====
    bool operator==(const FUUID& other) const;
    bool operator!=(const FUUID& other) const;
    bool operator<(const FUUID& other) const;

    // ===== Accessors =====
    uint64_t GetLow() const { return m_Low; }
    uint64_t GetHigh() const { return m_High; }

    // ===== Conversion =====
    std::string ToString() const;
    explicit operator std::string() const { return ToString(); }

    std::pair<uint64_t, uint64_t> ToPair() const { return { m_Low, m_High }; }

    // ===== Validation =====
    bool IsValid() const { return m_Low != 0 || m_High != 0; }
    static FUUID Invalid() { return FUUID(0, 0); }

    // ===== Static generators =====
    static FUUID Generate();
    static FUUID FromString(const std::string& str);

private:
    uint64_t m_Low;
    uint64_t m_High;
};

// Hash support for unordered containers
namespace std {
template <> struct hash<FUUID> {
    size_t operator()(const FUUID& uuid) const {
        uint64_t combined = uuid.GetLow() ^ uuid.GetHigh();
        return hash<uint64_t>()(combined);
    }
};
} // namespace std

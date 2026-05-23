#include <Core/Header/UUID.hpp>

#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace CZ {

// ===== Static random generator =====
static std::mt19937_64& GetRandomEngine() {
    static std::random_device rd;
    static std::mt19937_64 engine(rd());
    return engine;
}

// ===== Constructors =====
UUID::UUID() : m_Low(0), m_High(0) {}

UUID::UUID(uint64_t low, uint64_t high) : m_Low(low), m_High(high) {}

UUID::UUID(const std::string& str) { *this = FromString(str); }

// ===== Comparison =====
bool UUID::operator==(const UUID& other) const {
    return m_Low == other.m_Low && m_High == other.m_High;
}

bool UUID::operator!=(const UUID& other) const { return !(*this == other); }

bool UUID::operator<(const UUID& other) const {
    if (m_Low != other.m_Low) {
        return m_Low < other.m_Low;
    }
    return m_High < other.m_High;
}

// ===== Conversion =====
std::string UUID::ToString() const {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // Format: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    ss << std::setw(8) << (m_High >> 32);
    ss << "-";
    ss << std::setw(4) << ((m_High >> 16) & 0xFFFF);
    ss << "-";
    ss << std::setw(4) << (m_High & 0xFFFF);
    ss << "-";
    ss << std::setw(4) << ((m_Low >> 48) & 0xFFFF);
    ss << "-";
    ss << std::setw(12) << (m_Low & 0xFFFFFFFFFFFF);

    return ss.str();
}

// ===== Static generators =====
UUID UUID::Generate() {
    auto& engine = GetRandomEngine();
    std::uniform_int_distribution<uint64_t> dist;

    uint64_t high = dist(engine);
    uint64_t low  = dist(engine);

    // Set version (4) and variant bits
    high = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    low  = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    return UUID(low, high);
}

UUID UUID::FromString(const std::string& str) {
    // Expect format: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    if (str.length() != 36) {
        return Invalid();
    }

    uint64_t low = 0, high = 0;

    try {
        high |= static_cast<uint64_t>(std::stoull(str.substr(0, 8), nullptr, 16)) << 32;
        high |= static_cast<uint64_t>(std::stoull(str.substr(9, 4), nullptr, 16)) << 16;
        high |= static_cast<uint64_t>(std::stoull(str.substr(14, 4), nullptr, 16));

        low |= static_cast<uint64_t>(std::stoull(str.substr(19, 4), nullptr, 16)) << 48;
        low |= static_cast<uint64_t>(std::stoull(str.substr(24, 12), nullptr, 16));
    } catch (...) {
        return Invalid();
    }

    return UUID(low, high);
}

} // namespace CZ
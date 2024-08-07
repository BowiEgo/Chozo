#pragma once

#include <cstdint>

namespace Chozo {

    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }
        bool isValid() const { return m_UUID != std::numeric_limits<uint64_t>::max(); }

        static UUID Invalid() { return UUID(std::numeric_limits<uint64_t>::max()); }
    private:
        uint64_t m_UUID;
    };
}

namespace std {

    template<>
    struct hash<Chozo::UUID>
    {
        std::size_t operator()(const Chozo::UUID& uuid) const
        {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}

#pragma once

#include <vector>

namespace CZ {

class TypeMask {
public:
    static constexpr size_t BitsPerWord = 64;

    TypeMask() = default;

    void Clear() { m_Bits.clear(); }

    void Ensure(size_t index);
    void Set(size_t index);
    void Reset(size_t index);
    bool Test(size_t index) const;
    bool Any() const;
    std::vector<size_t> GetSetIndices() const;

    TypeMask& operator|=(const TypeMask& other);
    TypeMask operator|(const TypeMask& other) const;
    TypeMask operator&(const TypeMask& other) const;

private:
    std::vector<uint64_t> m_Bits;
};
} // namespace CZ

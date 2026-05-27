#include <Core/TypeRegistry/TypeMask.hpp>

namespace CZ {

void TypeMask::Ensure(size_t index) {
    size_t word = index / BitsPerWord;
    if (word >= m_Bits.size()) {
        m_Bits.resize(word + 1, 0);
    }
}

void TypeMask::Set(size_t index) {
    Ensure(index);
    m_Bits[index / BitsPerWord] |= (1ULL << (index % BitsPerWord));
}

void TypeMask::Reset(size_t index) {
    if (index / BitsPerWord < m_Bits.size()) {
        m_Bits[index / BitsPerWord] &= ~(1ULL << (index % BitsPerWord));
    }
}

bool TypeMask::Test(size_t index) const {
    size_t word = index / BitsPerWord;
    if (word >= m_Bits.size()) return false;
    return (m_Bits[word] & (1ULL << (index % BitsPerWord))) != 0;
}

bool TypeMask::Any() const {
    for (uint64_t word : m_Bits) {
        if (word != 0) return true;
    }
    return false;
}

std::vector<size_t> TypeMask::GetSetIndices() const {
    std::vector<size_t> indices;
    for (size_t wordIdx = 0; wordIdx < m_Bits.size(); ++wordIdx) {
        uint64_t word = m_Bits[wordIdx];
        while (word) {
            int bit = std::countr_zero(word); // C++20
            indices.push_back(wordIdx * 64 + bit);
            word &= word - 1;
        }
    }
    return indices;
}

TypeMask& TypeMask::operator|=(const TypeMask& other) {
    if (other.m_Bits.size() > m_Bits.size()) {
        m_Bits.resize(other.m_Bits.size(), 0);
    }
    for (size_t i = 0; i < other.m_Bits.size(); ++i) {
        m_Bits[i] |= other.m_Bits[i];
    }
    return *this;
}

TypeMask TypeMask::operator|(const TypeMask& other) const {
    TypeMask result = *this;
    result |= other;
    return result;
}

TypeMask TypeMask::operator&(const TypeMask& other) const {
    TypeMask result;
    size_t minSize = std::min(m_Bits.size(), other.m_Bits.size());
    result.m_Bits.resize(minSize, 0);
    for (size_t i = 0; i < minSize; ++i) {
        result.m_Bits[i] = m_Bits[i] & other.m_Bits[i];
    }
    return result;
}

} // namespace CZ

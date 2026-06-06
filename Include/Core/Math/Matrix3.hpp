#pragma once

#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector3.hpp>

namespace CZ {

class Matrix3 {
public:
    Matrix3() { SetIdentity(); }
    Matrix3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
            float m22) {
        m_Data[0] = m00;
        m_Data[1] = m01;
        m_Data[2] = m02;
        m_Data[3] = m10;
        m_Data[4] = m11;
        m_Data[5] = m12;
        m_Data[6] = m20;
        m_Data[7] = m21;
        m_Data[8] = m22;
    }
    explicit Matrix3(const Matrix4& m4) {
        const float* d = m4.Data();
        for (int i = 0; i < 9; ++i)
            m_Data[i] = d[i];
    }
    explicit Matrix3(const float* data) {
        for (int i = 0; i < 9; ++i)
            m_Data[i] = data[i];
    }

    const float* Data() const { return m_Data; }
    float* Data() { return m_Data; }

    float& operator[](int index) { return m_Data[index]; }
    const float& operator[](int index) const { return m_Data[index]; }

    float& operator()(int row, int col) { return m_Data[col * 3 + row]; }
    float operator()(int row, int col) const { return m_Data[col * 3 + row]; }

    Matrix3 operator*(const Matrix3& other) const;
    Vector3 operator*(const Vector3& v) const;

    Matrix3 Transpose() const;
    float Determinant() const;
    Matrix3 Inverse() const;
    void SetIdentity();
    std::string ToString() const;
    std::string ToStringCompact() const;

    static Matrix3 Identity();
    static Matrix3 RotationX(float angleDegrees);
    static Matrix3 RotationY(float angleDegrees);
    static Matrix3 RotationZ(float angleDegrees);
    static Matrix3 FromAxisAngle(const Vector3& axis, float angleDegrees);

private:
    float m_Data[9];
};

} // namespace CZ

// ===== String formatting for logs =====
// CZ_LOG(LogTemp, Info, "Matrix: {}", mat);     // Defaut
// CZ_LOG(LogTemp, Info, "Matrix: {:c}", mat);   // Compact
// CZ_LOG(LogTemp, Info, "Matrix: {:4}", mat);   // Specific precision
namespace fmt {
inline namespace v10 {
template <> struct formatter<CZ::Matrix3> {
    // Optional format specifiers
    int precision = 2;
    bool compact  = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();

        if (it == ctx.end() || *it == '}') {
            return it;
        }

        if (*it == 'c') {
            compact = true;
            ++it;
        } else if (*it >= '0' && *it <= '9') {
            precision = 0;
            while (it != ctx.end() && *it >= '0' && *it <= '9') {
                precision = precision * 10 + (*it - '0');
                ++it;
            }
        } else {
            throw format_error("invalid format specifier");
        }

        if (it == ctx.end() || *it != '}') {
            throw format_error("invalid format");
        }

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const CZ::Matrix3& mat, FormatContext& ctx) const {
        if (compact) {
            return fmt::format_to(ctx.out(), "{}", mat.ToStringCompact());
        }
        return fmt::format_to(ctx.out(), "{}", mat.ToString());
    }
};
} // namespace v10
} // namespace fmt

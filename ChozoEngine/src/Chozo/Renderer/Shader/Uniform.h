#pragma once

#include "czpch.h"
#include <glm/glm.hpp>
#include <any>

namespace Chozo {

    enum class UniformType {
        Bool,
        Int,
        Uint,
        Float,
        Float2,
        Float3,
        Float4,
        Vec2,
        Vec3,
        Vec4,
        Mat3,
        Mat4,
        Mat4Vec,
        IntVec,
        Unknown
    };

    using UniformValue = std::variant<
        bool,
        int,
        unsigned int,
        float,
        std::array<float, 2>,
        std::array<float, 3>,
        std::array<float, 4>,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat3,
        glm::mat4,
        std::vector<glm::mat4>,
        std::vector<int>
    >;

    namespace Uniform {
        static UniformType GetType(const UniformValue& value)
        {
            if (value.index() >= static_cast<size_t>(UniformType::Unknown)) {
                return UniformType::Unknown;
            }
            return static_cast<UniformType>(value.index());
        }

        static bool AsBool(const UniformValue& value)
        {
            if (const auto boolPtr = std::get_if<bool>(&value))
                return *boolPtr;

            if (const auto floatPtr = std::get_if<float>(&value))
                return static_cast<bool>(*floatPtr);

            if (const auto intPtr = std::get_if<int>(&value))
                return static_cast<bool>(*intPtr);

            if (const auto uintPtr = std::get_if<unsigned int>(&value))
                return static_cast<bool>(*uintPtr);

            throw std::bad_variant_access();
        }

        static int AsInt(const UniformValue& value)
        {
            if (const auto intPtr = std::get_if<int>(&value))
                return *intPtr;

            if (const auto uintPtr = std::get_if<unsigned int>(&value))
                return static_cast<int>(*uintPtr);

            if (const auto floatPtr = std::get_if<float>(&value))
                return static_cast<int>(*floatPtr);

            throw std::bad_variant_access();
        }

        static unsigned int AsUint(const UniformValue& value)
        {
            if (const auto uintPtr = std::get_if<unsigned int>(&value))
                return *uintPtr;

            if (const auto intPtr = std::get_if<int>(&value)) {
                if (*intPtr < 0) {
                    throw std::overflow_error("Cannot convert negative int to unsigned int");
                }
                return static_cast<unsigned int>(*intPtr);
            }

            if (const auto floatPtr = std::get_if<float>(&value)) {
                if (*floatPtr < 0.0f) {
                    throw std::overflow_error("Cannot convert negative float to unsigned int");
                }
                return static_cast<unsigned int>(*floatPtr);
            }

            throw std::bad_variant_access();
        }

        static float AsFloat(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<float>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<bool>(&value))
                return static_cast<float>(*ptr);

            if (const auto ptr = std::get_if<int>(&value))
                return static_cast<float>(*ptr);

            if (const auto ptr = std::get_if<unsigned int>(&value))
                return static_cast<float>(*ptr);

            throw std::bad_variant_access();
        }

        static std::array<float, 2> AsFloat2(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<std::array<float, 2>>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<glm::vec2>(&value))
                return std::array<float, 2>{ ptr->x, ptr->y };

            throw std::bad_variant_access();
        }

        static std::array<float, 3> AsFloat3(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<std::array<float, 3>>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<glm::vec3>(&value))
                return std::array<float, 3>{ ptr->x, ptr->y, ptr->z };

            throw std::bad_variant_access();
        }

        static std::array<float, 4> AsFloat4(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<std::array<float, 4>>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<glm::vec4>(&value))
                return std::array<float, 4>{ ptr->x, ptr->y , ptr->z , ptr->w };

            throw std::bad_variant_access();
        }

        static glm::vec2 AsVec2(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<glm::vec2>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<std::array<float, 2>>(&value))
                return { (*ptr)[0], (*ptr)[1] };

            throw std::bad_variant_access();
        }

        static glm::vec3 AsVec3(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<glm::vec3>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<std::array<float, 3>>(&value))
                return { (*ptr)[0], (*ptr)[1], (*ptr)[2] };

            throw std::bad_variant_access();
        }

        static glm::vec4 AsVec4(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<glm::vec4>(&value))
                return *ptr;

            if (const auto ptr = std::get_if<std::array<float, 4>>(&value))
                return { (*ptr)[0], (*ptr)[1], (*ptr)[2], (*ptr)[3] };

            throw std::bad_variant_access();
        }

        static glm::mat3 AsMat3(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<glm::mat3>(&value))
                return *ptr;

            throw std::bad_variant_access();
        }

        static glm::mat4 AsMat4(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<glm::mat4>(&value))
                return *ptr;

            throw std::bad_variant_access();
        }

        static std::vector<glm::mat4> AsMat4Vec(const UniformValue& value)
        {
            if (const auto ptr = std::get_if<std::vector<glm::mat4>>(&value))
                return *ptr;

            throw std::bad_variant_access();
        }

        static std::vector<int> AsIntVec(const UniformValue& value)
        {

            if (const auto ptr = std::get_if<std::vector<int>>(&value))
                return *ptr;

            throw std::bad_variant_access();
        }

        template <typename T>
        static T As(const UniformValue& value)
        {
            if (auto ptr = std::get_if<T>(&value)) {
                return *ptr;
            }

            if constexpr (std::is_same_v<T, bool>) {
                return AsBool(value);
            } else if constexpr (std::is_same_v<T, int>) {
                return AsInt(value);
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                return AsUint(value);
            } else if constexpr (std::is_same_v<T, float>) {
                return AsFloat(value);
            } else if constexpr (std::is_same_v<T, float[2]>) {
                return AsFloat2(value);
            } else if constexpr (std::is_same_v<T, float[3]>) {
                return AsFloat3(value);
            } else if constexpr (std::is_same_v<T, float[4]>) {
                return AsFloat4(value);
            } else if constexpr (std::is_same_v<T, glm::vec2>) {
                return AsVec2(value);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
                return AsVec3(value);
            } else if constexpr (std::is_same_v<T, glm::vec4>) {
                return AsVec4(value);
            } else if constexpr (std::is_same_v<T, glm::mat3>) {
                return AsMat3(value);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
                return AsMat4(value);
            } else if constexpr (std::is_same_v<T, std::vector<glm::mat4>>) {
                return AsMat4Vec(value);
            } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                return AsIntVec(value);
            }

            throw std::bad_variant_access();
        }
    }
}
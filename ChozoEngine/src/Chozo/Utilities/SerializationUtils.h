#pragma once

#include "Chozo/Renderer/Material.h"

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

// #define CZ_SERIALIZE_PROPERTY(propName, propVal, outputNode) outputNode << YAML::Key << propName << YAML::Value << propVal

namespace Chozo::Utils {

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::pair<float, float>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.first << v.second << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::tuple<float, float, float>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << std::get<0>(v) << std::get<1>(v) << std::get<2>(v) << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::tuple<float, float, float, float>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << std::get<0>(v) << std::get<1>(v) << std::get<2>(v) << std::get<3>(v) << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                out << v[i][j];
        out << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                out << v[i][j];
        out << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<glm::mat4>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (const auto& mat : v) {
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    out << mat[i][j];
        }
        out << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<int>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (const auto& val : v) {
            out << val;
        }
        out << YAML::EndSeq;
        return out;
    }
    
    namespace Serialization {

        static void SerializeProperty(std::string name, UniformValue value, YAML::Emitter& outputNode)
        {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, glm::mat3>)
                {}
                else if constexpr (std::is_same_v<T, glm::mat4>)
                {}
                else if constexpr (std::is_same_v<T, int>)
                {
                    outputNode << YAML::Key << name << YAML::Value << (arg ? "true" : "false");
                }
                else
                {
                    outputNode << YAML::Key << name << YAML::Value << arg;
                }
            }, value);
        }
    }

} // namespace Chozo

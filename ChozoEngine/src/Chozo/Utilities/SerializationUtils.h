#pragma once

#include "Chozo/Renderer/Shader/Uniform.h"

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

namespace Chozo::Utils {

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::array<float, 2>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v[0] << v[1] << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::array<float, 3>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v[0] << v[1] << v[2] << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::array<float, 4>& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v[0] << v[1] << v[2] << v[3] << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                out << v[i][j];
        out << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                out << v[i][j];
        out << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<glm::mat4>& v)
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

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::vector<int>& v)
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

        static void SerializeProperty(const std::string& name, const UniformValue& value, YAML::Emitter& outputNode)
        {
            switch (Uniform::GetType(value)) {
                case UniformType::Int:
                    outputNode << YAML::Key << name << YAML::Value << (Uniform::As<bool>(value) ? "true" : "false");
                    break;
                default:
                    std::visit([&](const auto& arg) {
                        outputNode << YAML::Key << name << YAML::Value << arg;
                    }, value);
                    break;
            }
        }
    }

} // namespace Chozo

#pragma once

#include "RendererTypes.h"
#include "Chozo/Core/Thread.h"

#include "czpch.h"
#include <glm/glm.hpp>

namespace Chozo {

    using UniformValue = std::variant<
        bool,
        int,
        unsigned int,
        float,
        std::pair<float, float>,
        std::tuple<float, float, float>,
        std::tuple<float, float, float, float>,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat3,
        glm::mat4,
        std::vector<glm::mat4>,
        std::vector<int>
    >;

    namespace Utils {

        static bool GetBool(const Chozo::UniformValue& value)
        {
            if (auto boolPtr = std::get_if<bool>(&value))
                return *boolPtr;
            
            if (auto floatPtr = std::get_if<float>(&value))
                return static_cast<bool>(*floatPtr);

            if (auto intPtr = std::get_if<int>(&value))
                return static_cast<bool>(*intPtr);

            if (auto uintPtr = std::get_if<unsigned int>(&value))
                return static_cast<bool>(*uintPtr);

            CZ_CORE_ERROR("UniformValue can't convert to type bool.");
            return false;
        }

        static float GetFloat(const Chozo::UniformValue& value)
        {
            if (auto floatPtr = std::get_if<float>(&value))
                return *floatPtr;

            if (auto boolPtr = std::get_if<bool>(&value))
                return static_cast<float>(*boolPtr);

            if (auto intPtr = std::get_if<int>(&value))
                return static_cast<float>(*intPtr);

            if (auto uintPtr = std::get_if<unsigned int>(&value))
                return static_cast<float>(*uintPtr);

            CZ_CORE_ERROR("UniformValue can't convert to type float.");
            return 0.0f;
        }

        static glm::vec3 GetVec3(const Chozo::UniformValue& value)
        {
            if (auto vec3Ptr = std::get_if<glm::vec3>(&value))
                return *vec3Ptr;

            if (auto tuplePtr = std::get_if<std::tuple<float, float, float>>(&value))
                return glm::vec3(std::get<0>(*tuplePtr), std::get<1>(*tuplePtr), std::get<2>(*tuplePtr));

            CZ_CORE_ERROR("UniformValue can't convert to type glm::vec3.");
            return glm::vec3(0.0f);
        }
    }
    
    class Shader : public RefCounted
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const std::string& GetName() const = 0;
        virtual const RendererID& GetRendererID() const = 0;

        virtual void SetUniform(const std::string& name, const UniformValue& value, const uint32_t count = 0) const = 0;
        virtual void ClearCache() = 0;
        virtual void Compile() = 0;
        virtual void AsyncCompile() = 0;

        static Ref<Shader> Create(const std::string& name, const std::vector<std::string> filePaths);
    };

    class ShaderLibrary : public RefCounted
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() override = default;

		void Load(std::string_view name, const std::vector<std::string> filePaths);
		void Recompile();

		const Ref<Shader>& Get(const std::string& name) const;

        static Ref<ShaderLibrary> Create();
    private:
        Thread m_Thread = Thread("ShaderLibrary");
        bool m_IsCompiling = false;
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };
}
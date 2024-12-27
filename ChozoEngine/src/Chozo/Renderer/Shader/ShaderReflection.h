#pragma once

#include "czpch.h"

namespace Chozo {

    struct UniformInfo
    {
        std::string type;
        std::string name;
        std::string resourceName;
        uint32_t size;
        uint32_t location;

        [[nodiscard]] std::string fullName() const { return resourceName + "." + name; }
    };

    struct AttributeInfo
    {
        std::string type;
        std::string name;
        uint32_t size;
        uint32_t location;
    };

    struct ShaderReflection
    {
        std::vector<UniformInfo> uniforms;
        std::vector<AttributeInfo> attributes;
        std::unordered_map<std::string, uint32_t> uniformLocations;
    };
}

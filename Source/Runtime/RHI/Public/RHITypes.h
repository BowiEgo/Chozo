#pragma once

#include "CoreTypes.h"
#include "RHIPCH.h"
#include <map>

////////////////////////////////////////////////////////////////////////////
//============================= Shader ===================================//
////////////////////////////////////////////////////////////////////////////

using FShaderID = uint32;

// -- EShaderStage --
// ENUM | LOWER | UPPER | SHORT | GLSL | VULKAN
#define FOREACH_SHADER_STAGE(TYPE)                                                                 \
    TYPE(Vertex, vertex, VERTEX, vert, vertex, Vertex)                                             \
    TYPE(Fragment, fragment, FRAGMENT, frag, fragment, Fragment)                                   \
    TYPE(Compute, compute, COMPUTE, comp, compute, Compute)                                        \
    TYPE(Geometry, geometry, GEOMETRY, geom, geometry, Geometry)                                   \
    TYPE(Hull, hull, HULL, tesc, tess_control, TessellationControl)                                \
    TYPE(Domain, domain, DOMAIN, tese, tess_evaluation, TessellationEvaluation)

enum class EShaderStage : uint16 {
#define GENERATE_ENUM(ENUM, ...) ENUM,
    FOREACH_SHADER_STAGE(GENERATE_ENUM)
#undef GENERATE_ENUM
        None
};

static constexpr size_t kShaderStageCount = static_cast<size_t>(EShaderStage::None);

// -- ShaderMacro --
struct ShaderMacro {
    std::string Name;
    std::string Definition;

    ShaderMacro() = default;
    ShaderMacro(const std::string& name, const std::string& def) : Name(name), Definition(def) {}
};

// A collection of macros, providing helper methods for hashing
// and string conversion
class FShaderMacros {
public:
    void Add(const std::string& name, const std::string& definition = "1") {
        m_Macros[name] = definition;
    }

    void Add(const std::map<std::string, std::string>& definitions) {
        for (const auto& [name, def] : definitions) {
            m_Macros[name] = def;
        }
    }

    // Convert to a string format for GLSL injection: "#define NAME DEF\n"
    std::string ToGLSLString() const {
        std::string result;
        for (const auto& [name, def] : m_Macros) {
            result += "#define " + name + " " + def + "\n";
        }
        return result;
    }

    // Generate a unique hash for shader permutation caching
    size_t GenHash() const {
        size_t hash = 0;
        for (const auto& [name, def] : m_Macros) {
            // Simple hash combine logic
            hash ^= std::hash<std::string>{}(name) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<std::string>{}(def) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }

    const std::map<std::string, std::string>& GetMap() const { return m_Macros; }

private:
    std::map<std::string, std::string> m_Macros;
};

struct FUniformInfo {
    std::string type;
    std::string name;
    std::string resourceName;
    uint32_t size;
    uint32_t location;

    std::string fullName() const { return resourceName + "." + name; }
};

struct FAttributeInfo {
    std::string type;
    std::string name;
    uint32_t size;
    uint32_t location;
};

struct FShaderReflection {
    std::vector<FUniformInfo> uniforms;
    std::vector<FAttributeInfo> attributes;
    std::unordered_map<std::string, uint32_t> uniformLocations;
};

// Define the environment and parameters for a single shader compilation task
struct FShaderCompilerInput {
    std::string VirtualPath;
    EShaderStage Stage;
    FShaderMacros Macros;
};

// The result of the compilation, including binaries and reflection data
struct FShaderCompilerOutput {
    std::vector<uint32_t> Binary;
    FShaderReflection Reflection;
    bool bSucceeded = false;
};

struct FExtent2D {
    uint32 Width;
    uint32 Height;
};

enum class EPixelFormat {
    Unknown,
    RGBA8_UNORM,
    RGBA8_SRGB,
    BGRA8_UNORM,
    BGRA8_SRGB,
    D32_SFLOAT,
    D24_UNORM_S8_UINT,
};

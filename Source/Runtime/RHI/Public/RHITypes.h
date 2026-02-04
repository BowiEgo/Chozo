#pragma once

#include "CoreMinimal.h"

////////////////////////////////////////////////////////////////////////////
//============================= Shader ===================================//
////////////////////////////////////////////////////////////////////////////

using FShaderID = uint32;

// -- EShaderStage --
// ENUM | LOWER | UPPER | SHORT | GLSL
#define FOREACH_SHADER_STAGE(TYPE)                                             \
    TYPE(Vertex, vertex, VERTEX, vert, vertex)                                 \
    TYPE(Fragment, fragment, FRAGMENT, frag, fragment)                         \
    TYPE(Compute, compute, COMPUTE, comp, compute)                             \
    TYPE(Geometry, geometry, GEOMETRY, geom, geometry)                         \
    TYPE(Hull, hull, HULL, tesc, tess_control)                                 \
    TYPE(Domain, domain, DOMAIN, tese, tess_evaluation)

enum class EShaderStage : uint16 {
#define GENERATE_ENUM(ENUM, ...) ENUM,
    FOREACH_SHADER_STAGE(GENERATE_ENUM)
#undef GENERATE_ENUM
        None
};

// -- ShaderMacro --
struct ShaderMacro {
    std::string Name;
    std::string Definition;

    ShaderMacro() = default;
    ShaderMacro(const std::string &name, const std::string &def)
        : Name(name), Definition(def) {}
};

// A collection of macros, providing helper methods for hashing
// and string conversion
class FShaderMacros {
public:
    void Add(const std::string &name, const std::string &definition = "1") {
        m_Macros[name] = definition;
    }

    void Add(const std::map<std::string, std::string> &definitions) {
        for (const auto &[name, def] : definitions) {
            m_Macros[name] = def;
        }
    }

    // Convert to a string format for GLSL injection: "#define NAME DEF\n"
    std::string ToGLSLString() const {
        std::string result;
        for (const auto &[name, def] : m_Macros) {
            result += "#define " + name + " " + def + "\n";
        }
        return result;
    }

    // Generate a unique hash for shader permutation caching
    size_t GetHash() const {
        size_t hash = 0;
        for (const auto &[name, def] : m_Macros) {
            // Simple hash combine logic
            hash ^= std::hash<std::string>{}(name) + 0x9e3779b9 + (hash << 6) +
                    (hash >> 2);
            hash ^= std::hash<std::string>{}(def) + 0x9e3779b9 + (hash << 6) +
                    (hash >> 2);
        }
        return hash;
    }

    const std::map<std::string, std::string> &GetMap() const {
        return m_Macros;
    }

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

using FShaderDefinitions = std::map<std::string, std::string>;

struct FShaderCreateInfo {
    std::string Name;
    std::string VirtualPath;
    std::string EntryPoint = "main";
    FShaderDefinitions
        Definitions; // Macros for permutations, e.g., {"USE_ALBEDO": "1"}

    FShaderCreateInfo(const std::string name, const std::string path,
                      const std::string entryPoint = "main",
                      const FShaderDefinitions &defs = {})
        : Name(name), VirtualPath(path), EntryPoint(entryPoint),
          Definitions(defs) {}

    size_t GetHash() const {
        size_t h = std::hash<std::string>{}(VirtualPath);

        auto hashCombine = [](size_t &seed, const std::string &s) {
            seed ^= std::hash<std::string>{}(s) + 0x9e3779b9 + (seed << 6) +
                    (seed >> 2);
        };

        for (const auto &[key, value] : Definitions) {
            hashCombine(h, key);
            hashCombine(h, value);
        }
        hashCombine(h, EntryPoint);

        return h;
    }
};

// Define the environment and parameters for a single shader compilation task
struct FShaderCompilerInput {
    std::string SourcePath;
    EShaderStage Stage;
    FShaderMacros Macros;
};

// The result of the compilation, including binaries and reflection data
struct FShaderCompilerOutput {
    std::vector<uint32_t> Binary;
    std::string SourceCode;
    FShaderReflection Reflection;
    bool bSucceeded = false;
};

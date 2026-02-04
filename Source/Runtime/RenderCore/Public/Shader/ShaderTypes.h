#pragma once

using FShaderID = uint32_t;

#define FOREACH_SHADER_STAGE(TYPE)                                             \
    TYPE(Vertex, vertex, VERTEX, vert)                                         \
    TYPE(Fragment, fragment, FRAGMENT, frag)

enum class ShaderStage : uint16_t {
#define GENERATE_ENUM(ENUM, LOWER_ENUM, UPPER_ENUM, SHORT_ENUM) ENUM,
    FOREACH_SHADER_STAGE(GENERATE_ENUM)
#undef GENERATE_ENUM
        None
};

inline static std::unordered_map<std::string, ShaderStage>
    s_ShaderExtensionMap = {
        // Vertex
        {".vert", ShaderStage::Vertex},

        // Fragment
        {".frag", ShaderStage::Fragment},
        {".pixel", ShaderStage::Fragment},
};

struct ShaderMacro {
    std::string Name;
    std::string Definition;

    ShaderMacro() = default;
    ShaderMacro(const std::string &name, const std::string &def)
        : Name(name), Definition(def) {}
};

// A collection of macros, providing helper methods for hashing
// and string conversion
class ShaderMacros {
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

struct UniformInfo {
    std::string type;
    std::string name;
    std::string resourceName;
    uint32_t size;
    uint32_t location;

    std::string fullName() const { return resourceName + "." + name; }
};

struct AttributeInfo {
    std::string type;
    std::string name;
    uint32_t size;
    uint32_t location;
};

struct FShaderReflection {
    std::vector<UniformInfo> uniforms;
    std::vector<AttributeInfo> attributes;
    std::unordered_map<std::string, uint32_t> uniformLocations;
};

using ShaderDefinitions = std::map<std::string, std::string>;

struct FShaderCreateInfo {
    std::string Name;
    std::string VirtualPath;
    std::string EntryPoint;
    ShaderDefinitions
        Definitions; // Macros for permutations, e.g., {"USE_ALBEDO": "1"}

    FShaderCreateInfo(const std::string name, const std::string path,
                      const std::string entryPoint = "main",
                      const ShaderDefinitions &defs = {})
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
    ShaderStage Stage;
    ShaderMacros Macros;
};

// The result of the compilation, including binaries and reflection data
struct FShaderCompilerOutput {
    std::vector<uint32_t> Binary;
    std::string SourceCode;
    FShaderReflection Reflection;
    bool bSucceeded = false;
};

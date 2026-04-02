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

struct FUniformSpecification {
    std::string type;
    std::string name;
    std::string resourceName;
    uint32 size;
    uint32 location;

    std::string fullName() const { return resourceName + "." + name; }
};

struct FAttributeInfo {
    std::string type;
    std::string name;
    uint32 size;
    uint32 location;
};

struct FShaderReflection {
    std::vector<FUniformSpecification> uniforms;
    std::vector<FAttributeInfo> attributes;
    std::unordered_map<std::string, uint32> uniformLocations;
};

// Define the environment and parameters for a single shader compilation task
struct FShaderCompilerInput {
    std::string VirtualPath;
    EShaderStage Stage;
    FShaderMacros Macros;
};

// The result of the compilation, including binaries and reflection data
struct FShaderCompilerOutput {
    std::vector<uint32> Binary;
    FShaderReflection Reflection;
    bool bSucceeded = false;
};

struct FExtent2D {
    uint32 Width;
    uint32 Height;
};

// clang-format off
enum class EPixelFormat {
    Unknown,   // Undefined or invalid format

    // --- Single-channel (R) ---
    R8_UNORM,   // 8-bit unsigned normalized
    R16_UNORM,  // 16-bit unsigned normalized
    R16F,       // 16-bit float
    R32F,       // 32-bit float

    // --- Dual-channel (RG) ---
    RG8_UNORM,  // 8-bit unsigned normalized per channel
    RG16_UNORM, // 16-bit unsigned normalized per channel
    RG16F,      // 16-bit float per channel
    RG32F,      // 32-bit float per channel

    // --- 8-bit 4-channel (RGBA/BGRA) ---
    RGBA8_UNORM, // 8-bit unsigned normalized, linear
    RGBA8_SRGB,  // 8-bit sRGB (gamma-corrected)
    BGRA8_UNORM, // 8-bit unsigned normalized, BGR order, linear
    BGRA8_SRGB,  // 8-bit sRGB, BGR order

    // --- 16-bit 4-channel ---
    RGBA16_UNORM, // 16-bit unsigned normalized per channel
    RGBA16F,      // 16-bit float per channel

    // --- 32-bit 4-channel ---
    RGBA32F,      // 32-bit float per channel

    // --- Special packed RGB formats (no alpha) ---
    RGB9E5,       // 9:9:9:5 shared exponent, RGB
    R11G11B10F,   // 11:11:10 float, no sign, RGB

    // --- Depth / Stencil formats ---
    D16_UNORM,          // 16-bit unsigned normalized depth
    D24_UNORM_S8_UINT,  // 24-bit depth + 8-bit stencil
    D32_SFLOAT          // 32-bit float depth
};
// clang-format on

enum class ETextureUsage {
    None = 0,
    Texture,         // For shader reading (e.g., Albedo maps)
    ColorAttachment, // For rendering targets
    DepthAttachment, // For rendering targets
    Storage          // For Compute Shader read/write (RWTexture)
};

enum class EPresentMode {
    Immediate = 0, // VSync OFF. Minimal latency, high tearing.
    Mailbox,       // VSync OFF (Fast). No tearing, but heavy GPU load.
    FIFO,          // VSync ON. Standard synchronized presentation.
    FIFO_Relaxed,  // VSync ON (Adaptive). Latency-optimized.

    Unkown
};

enum class EPolygonMode { Fill, Line, Point };

/**
 * ECommandPoolFlags - Maps to underlying API flags (e.g., VkCommandPoolCreateFlagBits).
 * Defines the memory allocation behavior and reset capabilities of the pool.
 */
enum class ECommandPoolFlags : uint32 {
    None = 0,

    /** * Indicates that command buffers allocated from the pool are short-lived
     * and will be reset or freed in a relatively short period.
     * Maps to VK_COMMAND_POOL_CREATE_TRANSIENT_BIT.
     */
    Transient = 1 << 0,

    /** * Allows command buffers allocated from this pool to be reset individually.
     * Without this flag, the entire pool must be reset to reuse memory.
     * Maps to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT.
     */
    ResetCommandBuffer = 1 << 1,

    /** * Allocated command buffers will be protected from unauthorized access.
     * Maps to VK_COMMAND_POOL_CREATE_PROTECTED_BIT.
     */
    Protected = 1 << 2,

    // Default configuration for most rendering scenarios
    Default = ResetCommandBuffer
};

// Enable bitwise operations for the enum class
ENUM_CLASS_FLAGS(ECommandPoolFlags);

// ===== Buffer Usage Flags =====
enum class EBufferUsage : uint32 {
    None = 0,
    TransferSrc = 1 << 0,        // VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    TransferDst = 1 << 1,        // VK_BUFFER_USAGE_TRANSFER_DST_BIT
    UniformTexelBuffer = 1 << 2, // VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
    StorageTexelBuffer = 1 << 3, // VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    UniformBuffer = 1 << 4,      // VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    StorageBuffer = 1 << 5,      // VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    IndexBuffer = 1 << 6,        // VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    VertexBuffer = 1 << 7,       // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    IndirectBuffer = 1 << 8,     // VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
    AccelerationStructure =
        1 << 9, // VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
    ShaderDeviceAddress = 1 << 10, // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
};

inline EBufferUsage operator|(EBufferUsage a, EBufferUsage b) {
    return static_cast<EBufferUsage>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline EBufferUsage operator&(EBufferUsage a, EBufferUsage b) {
    return static_cast<EBufferUsage>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

inline EBufferUsage& operator|=(EBufferUsage& a, EBufferUsage b) {
    a = a | b;
    return a;
}

// ===== Memory Type Flags =====
enum class EMemoryType : uint32 {
    Unknown = 0,
    DeviceLocal = 1 << 0,     // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    HostVisible = 1 << 1,     // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    HostCoherent = 1 << 2,    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    HostCached = 1 << 3,      // VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    LazilyAllocated = 1 << 4, // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
    Protected = 1 << 5,       // VK_MEMORY_PROPERTY_PROTECTED_BIT
    DeviceAddress = 1 << 6,   // VK_MEMORY_PROPERTY_DEVICE_ADDRESS_BIT
};

inline EMemoryType operator|(EMemoryType a, EMemoryType b) {
    return static_cast<EMemoryType>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline EMemoryType operator&(EMemoryType a, EMemoryType b) {
    return static_cast<EMemoryType>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

inline bool HasFlag(EMemoryType value, EMemoryType flag) {
    return (static_cast<uint32>(value) & static_cast<uint32>(flag)) != 0;
}

inline bool HasFlag(EBufferUsage value, EBufferUsage flag) {
    return (static_cast<uint32>(value) & static_cast<uint32>(flag)) != 0;
}

// ===== Shader =====
enum class EShaderDataType {
    None = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    Int,
    Int2,
    Int3,
    Int4,
    Bool
};

static uint32 FShaderDataTypeSize(EShaderDataType type) {
    switch (type) {
        case EShaderDataType::None: return 0;
        case EShaderDataType::Float: return 4;
        case EShaderDataType::Float2: return 4 * 2;
        case EShaderDataType::Float3: return 4 * 3;
        case EShaderDataType::Float4: return 4 * 4;
        case EShaderDataType::Mat3: return 4 * 3 * 3;
        case EShaderDataType::Mat4: return 4 * 4 * 4;
        case EShaderDataType::Int: return 4;
        case EShaderDataType::Int2: return 4 * 2;
        case EShaderDataType::Int3: return 4 * 3;
        case EShaderDataType::Int4: return 4 * 4;
        case EShaderDataType::Bool: return 1;
    }

    // CZ_CORE_ASSERT(false, "Unknown EShaderDataType!");
    return 0;
}

struct FBufferElement {
    std::string Name;
    EShaderDataType Type;
    uint32 Size;
    uint32 Offset;
    bool Normalized;

    FBufferElement() {}

    FBufferElement(EShaderDataType type, const std::string& name, bool normalized = false)
        : Name(name), Type(type), Size(FShaderDataTypeSize(type)), Offset(0),
          Normalized(normalized) {}

    uint32 GetComponentCount() const {
        switch (Type) {
            case EShaderDataType::None: return 0;
            case EShaderDataType::Float: return 1;
            case EShaderDataType::Float2: return 2;
            case EShaderDataType::Float3: return 3;
            case EShaderDataType::Float4: return 4;
            case EShaderDataType::Mat3: return 3 * 3;
            case EShaderDataType::Mat4: return 4 * 4;
            case EShaderDataType::Int: return 1;
            case EShaderDataType::Int2: return 2;
            case EShaderDataType::Int3: return 3;
            case EShaderDataType::Int4: return 4;
            case EShaderDataType::Bool: return 1;
        }

        // CZ_CORE_ASSERT(false, "Unknown EShaderDataType!");
        return 0;
    }
};

class VertexBufferLayout {
private:
    std::vector<FBufferElement> m_Elements;
    uint32 m_Stride = 0;

private:
    void CalculateOffsetsAndStride() {
        uint32 offset = 0;
        m_Stride = 0;
        for (auto& element : m_Elements) {
            element.Offset = offset;
            offset += element.Size;
            m_Stride += element.Size;
        }
    }

public:
    VertexBufferLayout() {}

    VertexBufferLayout(const std::initializer_list<FBufferElement>& elements)
        : m_Elements(elements) {
        CalculateOffsetsAndStride();
    }

    inline uint32 GetStride() const { return m_Stride; }
    inline const std::vector<FBufferElement>& GetElements() const { return m_Elements; }

    std::vector<FBufferElement>::iterator begin() { return m_Elements.begin(); }
    std::vector<FBufferElement>::iterator end() { return m_Elements.end(); }
    std::vector<FBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
    std::vector<FBufferElement>::const_iterator end() const { return m_Elements.end(); }
};

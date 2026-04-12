#pragma once

#include "CoreTypes.h"
#include "RHIPCH.h"

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

// enum class EShaderStage : uint16 {
// #define GENERATE_ENUM(ENUM, ...) ENUM,
//     FOREACH_SHADER_STAGE(GENERATE_ENUM)
// #undef GENERATE_ENUM
//         None
// };

// static constexpr size_t kShaderStageCount = static_cast<size_t>(EShaderStage::None);

enum class EShaderStageIndex : uint16 {
#define GENERATE_INDEX(ENUM, ...) ENUM,
    FOREACH_SHADER_STAGE(GENERATE_INDEX)
#undef GENERATE_INDEX
        Count
};

enum class EShaderStage : uint16 {
    None = 0,
#define GENERATE_BIT(ENUM, ...) ENUM = 1 << static_cast<uint16>(EShaderStageIndex::ENUM),
    FOREACH_SHADER_STAGE(GENERATE_BIT)
#undef GENERATE_BIT
    // clang-format off
    AllGraphics = (Vertex | Fragment | Geometry | Hull | Domain)
    // clang-format on

};
ENUM_CLASS_FLAGS(EShaderStage);

static constexpr size_t kShaderStageCount = static_cast<size_t>(EShaderStageIndex::Count);

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

enum class EShaderDataFormat {
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
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Bool
};

inline const char* ShaderDataFormatToString(EShaderDataFormat format) {
    switch (format) {
        case EShaderDataFormat::Float: return "Float";
        case EShaderDataFormat::Float2: return "Float2";
        case EShaderDataFormat::Float3: return "Float3";
        case EShaderDataFormat::Float4: return "Float4";
        case EShaderDataFormat::Mat3: return "Mat3";
        case EShaderDataFormat::Mat4: return "Mat4";
        case EShaderDataFormat::Int: return "Int";
        case EShaderDataFormat::Int2: return "Int2";
        case EShaderDataFormat::Int3: return "Int3";
        case EShaderDataFormat::Int4: return "Int4";
        case EShaderDataFormat::UInt: return "UInt";
        case EShaderDataFormat::UInt2: return "UInt2";
        case EShaderDataFormat::UInt3: return "UInt3";
        case EShaderDataFormat::UInt4: return "UInt4";
        case EShaderDataFormat::Bool: return "Bool";
        case EShaderDataFormat::None:
        default: return "None/Unknown";
    }
}

static uint32 FShaderDataTypeSize(EShaderDataFormat type) {
    switch (type) {
        case EShaderDataFormat::None: return 0;

        // --- Floating Point ---
        case EShaderDataFormat::Float: return 4;
        case EShaderDataFormat::Float2: return 4 * 2;
        case EShaderDataFormat::Float3: return 4 * 3;
        case EShaderDataFormat::Float4: return 4 * 4;

        // --- Matrices (Logical size) ---
        case EShaderDataFormat::Mat3: return 4 * 3 * 3; // 36 bytes
        case EShaderDataFormat::Mat4:
            return 4 * 4 * 4; // 64 bytes

        // --- Signed Integers ---
        case EShaderDataFormat::Int: return 4;
        case EShaderDataFormat::Int2: return 4 * 2;
        case EShaderDataFormat::Int3: return 4 * 3;
        case EShaderDataFormat::Int4: return 4 * 4;

        // --- Unsigned Integers ---
        case EShaderDataFormat::UInt: return 4;
        case EShaderDataFormat::UInt2: return 4 * 2;
        case EShaderDataFormat::UInt3: return 4 * 3;
        case EShaderDataFormat::UInt4: return 4 * 4;

        // --- Boolean ---
        case EShaderDataFormat::Bool: return 1;
    }

    // CZ_CORE_ASSERT(false, "Unknown EShaderDataFormat!");
    return 0;
}

enum class EUniformType : uint8_t {
    None = 0,
    Sampler,
    Image,
    CombinedImageSampler,
    UniformBuffer,
    StorageImage,
    StorageBuffer,
    PushConstant,
    InputAttachment // subpassInput (use for deferred rendering)
};

struct FUniformSpecification {
    EUniformType Type;
    EShaderDataFormat Format;
    std::string Name;         // 成员名 (如 "u_Color")
    std::string ResourceName; // 容器名 (如 "u_MaterialData")
    uint32 Size      = 0;     // 字节大小
    uint32 Offset    = 0;
    uint32 Binding   = 0; // 绑定点
    uint32 Set       = 0; // 所在的 DescriptorSet 编号
    uint32 ArraySize = 1; // 数组长度 (1 表示非数组，0 可能表示运行时数组)

    std::string fullName() const {
        return ResourceName.empty() ? Name : (ResourceName + "." + Name);
    }

    std::string ToString() const {
        std::stringstream ss;
        if (Type == EUniformType::PushConstant) {
            ss << "[PushConstant] " << Name << " (Size: " << Size << ")";
        } else {
            ss << "[Set " << Set << ", Binding " << Binding << "] " << Name
               << " (Type: " << (uint32)Type << ", Size: " << Size << ", Offset: " << Offset
               << ", Array: " << ArraySize << ")";
        }
        return ss.str();
    }
};

struct FAttributeInfo {
    std::string Name;
    uint32 Location = 0;
    uint32 Size     = 0;
    EShaderDataFormat Format;

    FAttributeInfo() = default;

    // Logic-driven constructor
    FAttributeInfo(const std::string& name, uint32 loc, EShaderDataFormat format)
        : Name(name), Location(loc), Format(format) {
        Size = FShaderDataTypeSize(format);
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << "[Loc " << Location << "] " << Name
           << " (Format: " << ShaderDataFormatToString(Format) << ", Size: " << Size << " bytes)";
        return ss.str();
    }
};

struct FShaderReflection {
    std::vector<FUniformSpecification> Uniforms;
    std::vector<FAttributeInfo> Attributes;
    std::unordered_map<std::string, uint32> UniformLocations;

    std::string ToString() const {
        std::stringstream ss;
        ss << "--- Shader Reflection Report ---\n";

        ss << " [Input Attributes]\n";
        if (Attributes.empty()) ss << "   (None)\n";
        for (const auto& attr : Attributes) {
            ss << "   " << attr.ToString() << "\n";
        }

        ss << " [Resource Bindings]\n";
        if (Uniforms.empty()) ss << "   (None)\n";
        for (const auto& uni : Uniforms) {
            ss << "   " << uni.ToString() << "\n";
        }

        // Optional: Only show the map if it's not redundant
        if (!UniformLocations.empty()) {
            ss << " [Name-to-Location Map]\n";
            for (const auto& [name, loc] : UniformLocations) {
                ss << "   " << name << " -> " << loc << "\n";
            }
        }

        ss << "--------------------------------";
        return ss.str();
    }
};

struct FRHIShaderResourceBinding {
    uint32_t Binding;
    EUniformType Type;
    uint32_t DescriptorCount;
    EShaderStage StageFlags;

    bool operator==(const FRHIShaderResourceBinding& other) const {
        return Binding == other.Binding && Type == other.Type &&
               DescriptorCount == other.DescriptorCount && StageFlags == other.StageFlags;
    }
};

struct FRHISetLayoutDescription {
    std::vector<FRHIShaderResourceBinding> Bindings;

    void AddBinding(const uint32_t binding, const EUniformType type, const uint32_t descriptorCount,
                    const EShaderStage stage) {
        Bindings.push_back(FRHIShaderResourceBinding(binding, type, descriptorCount, stage));
    }

    size_t GetHash() const {
        size_t h = 0;
        for (const auto& b : Bindings) {
            HashCombine(h, std::hash<uint32_t>{}(b.Binding));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.Type)));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.StageFlags)));
        }
        return h;
    }

    bool operator==(const FRHISetLayoutDescription& other) const {
        return Bindings == other.Bindings;
    }
};

struct FRHIPushConstantRange {
    EShaderStage StageFlags;
    uint32_t Offset;
    uint32_t Size;
};

struct FRHIPipelineLayoutDescription {
    std::map<uint32_t, FRHISetLayoutDescription> SetLayouts;
    std::vector<FRHIPushConstantRange> PushConstantRanges;

    bool operator==(const FRHIPipelineLayoutDescription& other) const;
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

struct FBufferElement {
    std::string Name;
    EShaderDataFormat Type;
    uint32 Size;
    uint32 Offset;
    bool Normalized;

    FBufferElement() {}

    FBufferElement(EShaderDataFormat type, const std::string& name, bool normalized = false)
        : Name(name), Type(type), Size(FShaderDataTypeSize(type)), Offset(0),
          Normalized(normalized) {}

    uint32 GetComponentCount() const {
        switch (Type) {
            case EShaderDataFormat::None: return 0;

            // --- Floats & Mats ---
            case EShaderDataFormat::Float: return 1;
            case EShaderDataFormat::Float2: return 2;
            case EShaderDataFormat::Float3: return 3;
            case EShaderDataFormat::Float4: return 4;
            case EShaderDataFormat::Mat3: return 3 * 3; // 9
            case EShaderDataFormat::Mat4:
                return 4 * 4; // 16

            // --- Signed Integers ---
            case EShaderDataFormat::Int: return 1;
            case EShaderDataFormat::Int2: return 2;
            case EShaderDataFormat::Int3: return 3;
            case EShaderDataFormat::Int4: return 4;

            // --- Unsigned Integers ---
            case EShaderDataFormat::UInt: return 1;
            case EShaderDataFormat::UInt2: return 2;
            case EShaderDataFormat::UInt3: return 3;
            case EShaderDataFormat::UInt4: return 4;

            // --- Boolean ---
            case EShaderDataFormat::Bool: return 1;
        }

        // CZ_CORE_ASSERT(false, "Unknown EShaderDataFormat!");
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
        m_Stride      = 0;
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

enum class EFilter : uint8_t {
    Nearest = 0,
    Linear  = 1,

    Default = Linear
};

enum class EAddressMode : uint8_t {
    Repeat            = 0,
    MirroredRepeat    = 1,
    ClampToEdge       = 2,
    ClampToBorder     = 3,
    MirrorClampToEdge = 4,

    Default = Repeat
};

/**
 * @brief Defines how the sampler interpolates between mipmap levels.
 */
enum class EMipmapMode : uint8_t {
    Nearest = 0,
    Linear  = 1,

    Default = Linear
};

enum class EImageUsage : uint32_t {
    Sampled             = 1 << 0, // For shader reading (e.g., texture sampling)
    ColorAttachment     = 1 << 1, // For framebuffer color attachments
    DepthStencil        = 1 << 2, // For framebuffer depth/stencil attachments
    Storage             = 1 << 3, // For Compute Shader read/write (RWTexture)
    TransferSrc         = 1 << 4, // For Copy Source (e.g., Readback or Mip Generation)
    TransferDst         = 1 << 5, // For Copy Destination (e.g., Upload or Mip Generation)
    TransientAttachment = 1 << 6  // For transient resources optimization
};
ENUM_CLASS_FLAGS(EImageUsage);

enum class EImageLayout {
    Undefined,
    General,
    ColorAttachmentOptimal,
    DepthStencilAttachmentOptimal,
    DepthStencilReadOnlyOptimal,
    ShaderReadOnlyOptimal,
    TransferSrcOptimal,
    TransferDstOptimal,
    PresentSrc,

    // Add more as needed for specific APIs (e.g., Vulkan has many more)
    Unknown
};

enum class EImageViewType {
    View1D = 0,
    View2D,
    View3D,
    ViewCube,
    View1DArray,
    View2DArray,
    ViewCubeArray
};

enum class ETextureType { Texture2D, TextureCube };

enum class ETextureUsage : uint32_t {
    None       = 0,
    Texture    = 1 << 0, // Regular sampled texture (e.g., Shader Resource like Albedo maps)
    Attachment = 1 << 1, // Rendering target (Color/Depth/Stencil)
    Storage    = 1 << 2, // Compute shader read/write (RWTexture)
    Upload     = 1 << 3, // Needs to be updated from CPU
    Readback   = 1 << 4  // Needs to be read back to CPU
};
ENUM_CLASS_FLAGS(ETextureUsage);

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
    None               = 0,
    TransferSrc        = 1 << 0, // VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    TransferDst        = 1 << 1, // VK_BUFFER_USAGE_TRANSFER_DST_BIT
    UniformTexelBuffer = 1 << 2, // VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
    StorageTexelBuffer = 1 << 3, // VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    UniformBuffer      = 1 << 4, // VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    StorageBuffer      = 1 << 5, // VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    IndexBuffer        = 1 << 6, // VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    VertexBuffer       = 1 << 7, // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    IndirectBuffer     = 1 << 8, // VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
    AccelerationStructure =
        1 << 9, // VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
    ShaderDeviceAddress = 1 << 10, // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
};
ENUM_CLASS_FLAGS(EBufferUsage);

// ===== Memory Type Flags =====
enum class EMemoryType : uint32 {
    Unknown         = 0,
    DeviceLocal     = 1 << 0, // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    HostVisible     = 1 << 1, // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    HostCoherent    = 1 << 2, // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    HostCached      = 1 << 3, // VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    LazilyAllocated = 1 << 4, // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
    Protected       = 1 << 5, // VK_MEMORY_PROPERTY_PROTECTED_BIT
    DeviceAddress   = 1 << 6, // VK_MEMORY_PROPERTY_DEVICE_ADDRESS_BIT
};
ENUM_CLASS_FLAGS(EMemoryType);

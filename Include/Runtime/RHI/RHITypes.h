#pragma once

#include <Core/Header/Enum.h>
#include <Core/Header/Macros.h>
#include <Core/Header/Types.h>

#include <sstream>
#include <unordered_map>

namespace CZ {

////////////////////////////////////////////////////////////////////////////
//============================= Shader ===================================//
////////////////////////////////////////////////////////////////////////////

using ShaderID = uint32;

// -- ShaderStage --
// ENUM | LOWER | UPPER | SHORT | GLSL | VULKAN | VULKAN_UPPER
#define FOREACH_SHADER_STAGE(TYPE)                                                                 \
    TYPE(Vertex, vertex, VERTEX, vert, vertex, Vertex, VERTEX)                                     \
    TYPE(Fragment, fragment, FRAGMENT, frag, fragment, Fragment, FRAGMENT)                         \
    TYPE(Compute, compute, COMPUTE, comp, compute, Compute, COMPUTE)                               \
    TYPE(Geometry, geometry, GEOMETRY, geom, geometry, Geometry, GEOMETRY)                         \
    TYPE(Hull, hull, HULL, tesc, tess_control, TessellationControl, TESSELLATION_CONTROL)          \
    TYPE(Domain, domain, DOMAIN, tese, tess_evaluation, TessellationEvaluation,                    \
         TESSELLATION_EVALUATION)

// enum class ShaderStage : uint16 {
// #define GENERATE_ENUM(ENUM, ...) ENUM,
//     FOREACH_SHADER_STAGE(GENERATE_ENUM)
// #undef GENERATE_ENUM
//         None
// };

// static constexpr size_t kShaderStageCount = static_cast<size_t>(ShaderStage::None);

enum class ShaderStageIndex : uint16 {
#define GENERATE_INDEX(ENUM, ...) ENUM,
    FOREACH_SHADER_STAGE(GENERATE_INDEX)
#undef GENERATE_INDEX
        Count
};

enum class ShaderStage : uint16 {
    None = 0,
#define GENERATE_BIT(ENUM, ...) ENUM = 1 << static_cast<uint16>(ShaderStageIndex::ENUM),
    FOREACH_SHADER_STAGE(GENERATE_BIT)
#undef GENERATE_BIT
    // clang-format off
    AllGraphics = (Vertex | Fragment | Geometry | Hull | Domain)
    // clang-format on

};
ENUM_CLASS_FLAGS(ShaderStage);

static constexpr size_t kShaderStageCount = static_cast<size_t>(ShaderStageIndex::Count);

// -- ShaderMacro --
struct ShaderMacro {
    std::string Name;
    std::string Definition;

    ShaderMacro() = default;
    ShaderMacro(const std::string& name, const std::string& def) : Name(name), Definition(def) {}
};

// A collection of macros, providing helper methods for hashing
// and string conversion
class ShaderMacros {
public:
    void Add(const std::string& name, const std::string& definition = "1") {
        m_Macros[name] = definition;
    }

    void Add(const std::unordered_map<std::string, std::string>& definitions) {
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

    const std::unordered_map<std::string, std::string>& GetMap() const { return m_Macros; }

private:
    std::unordered_map<std::string, std::string> m_Macros;
};

enum class ShaderDataType {
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

inline const char* ShaderDataTypeToString(ShaderDataType format) {
    switch (format) {
        case ShaderDataType::Float: return "Float";
        case ShaderDataType::Float2: return "Float2";
        case ShaderDataType::Float3: return "Float3";
        case ShaderDataType::Float4: return "Float4";
        case ShaderDataType::Mat3: return "Mat3";
        case ShaderDataType::Mat4: return "Mat4";
        case ShaderDataType::Int: return "Int";
        case ShaderDataType::Int2: return "Int2";
        case ShaderDataType::Int3: return "Int3";
        case ShaderDataType::Int4: return "Int4";
        case ShaderDataType::UInt: return "UInt";
        case ShaderDataType::UInt2: return "UInt2";
        case ShaderDataType::UInt3: return "UInt3";
        case ShaderDataType::UInt4: return "UInt4";
        case ShaderDataType::Bool: return "Bool";
        case ShaderDataType::None:
        default: return "None/Unknown";
    }
}

static uint32 ShaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::None: return 0;

        // --- Floating Point ---
        case ShaderDataType::Float: return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;

        // --- Matrices (Logical size) ---
        case ShaderDataType::Mat3: return 4 * 3 * 3; // 36 bytes
        case ShaderDataType::Mat4:
            return 4 * 4 * 4; // 64 bytes

        // --- Signed Integers ---
        case ShaderDataType::Int: return 4;
        case ShaderDataType::Int2: return 4 * 2;
        case ShaderDataType::Int3: return 4 * 3;
        case ShaderDataType::Int4: return 4 * 4;

        // --- Unsigned Integers ---
        case ShaderDataType::UInt: return 4;
        case ShaderDataType::UInt2: return 4 * 2;
        case ShaderDataType::UInt3: return 4 * 3;
        case ShaderDataType::UInt4: return 4 * 4;

        // --- Boolean ---
        case ShaderDataType::Bool: return 1;
    }

    // CZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

enum class UniformType : uint8_t {
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

struct UniformSpecification {
    UniformType Type;
    ShaderDataType Format;
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
        if (Type == UniformType::PushConstant) {
            ss << "[PushConstant] " << Name << " (Size: " << Size << ")";
        } else {
            ss << "[Set " << Set << ", Binding " << Binding << "] " << Name
               << " (Type: " << (uint32)Type << ", Size: " << Size << ", Offset: " << Offset
               << ", Array: " << ArraySize << ")";
        }
        return ss.str();
    }
};

struct AttributeInfo {
    std::string Name;
    uint32 Location = 0;
    uint32 Size     = 0;
    ShaderDataType Type;

    AttributeInfo() = default;

    // Logic-driven constructor
    AttributeInfo(const std::string& name, uint32 loc, ShaderDataType type)
        : Name(name), Location(loc), Type(type) {
        Size = ShaderDataTypeSize(type);
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << "[Loc " << Location << "] " << Name << " (Type: " << ShaderDataTypeToString(Type)
           << ", Size: " << Size << " bytes)";
        return ss.str();
    }
};

struct ShaderReflection {
    std::vector<UniformSpecification> Uniforms;
    std::vector<AttributeInfo> Attributes;
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

struct RHIShaderResourceBinding {
    uint32_t Binding;
    UniformType Type;
    uint32_t DescriptorCount;
    ShaderStage StageFlags;

    bool operator==(const RHIShaderResourceBinding& other) const {
        return Binding == other.Binding && Type == other.Type &&
               DescriptorCount == other.DescriptorCount && StageFlags == other.StageFlags;
    }
};

struct FRHISetLayoutDescription {
    std::vector<RHIShaderResourceBinding> Bindings;

    void AddBinding(const uint32_t binding, const UniformType type, const uint32_t descriptorCount,
                    const ShaderStage stage) {
        Bindings.push_back(RHIShaderResourceBinding(binding, type, descriptorCount, stage));
    }

    size_t GetHash() const {
        size_t h = 0;
        for (const auto& b : Bindings) {
            HashCombine(h, std::hash<uint32_t>{}(b.Binding));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.Type)));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.DescriptorCount)));
            HashCombine(h, std::hash<uint32_t>{}(static_cast<uint32_t>(b.StageFlags)));
        }
        return h;
    }

    bool operator==(const FRHISetLayoutDescription& other) const {
        return Bindings == other.Bindings;
    }
};

struct PushConstantRange {
    ShaderStage StageFlags;
    uint32_t Offset;
    uint32_t Size;
};

struct PipelineLayoutDescription {
    std::unordered_map<uint32_t, FRHISetLayoutDescription> SetLayouts;
    std::vector<PushConstantRange> PushConstantRanges;

    bool operator==(const PipelineLayoutDescription& other) const;
};

// Define the environment and parameters for a single shader compilation task
struct ShaderCompilerInput {
    std::string VirtualPath;
    ShaderStage Stage;
    ShaderMacros Macros;
};

struct ShaderCompilerMultiInput {
    std::string VirtualPath;
    std::vector<ShaderStage> Stages;
    ShaderMacros Macros;
};

// The result of the compilation, including binaries and reflection data
struct ShaderCompilerOutput {
    std::vector<uint32> Binary;
    ShaderReflection Reflection;
    bool bSucceeded = false;
};

struct BufferElement {
    ShaderDataType Type;
    std::string Name;
    uint32 Size;
    uint32 Offset;
    uint32 Location;
    bool Normalized;

    BufferElement() {}

    BufferElement(ShaderDataType type, const std::string& name, uint32 offset = 0,
                  uint32 location = 0, bool normalized = false)
        : Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(offset),
          Location(location), Normalized(normalized) {}

    uint32 GetComponentCount() const {
        switch (Type) {
            case ShaderDataType::None: return 0;

            // --- Floats & Mats ---
            case ShaderDataType::Float: return 1;
            case ShaderDataType::Float2: return 2;
            case ShaderDataType::Float3: return 3;
            case ShaderDataType::Float4: return 4;
            case ShaderDataType::Mat3: return 3 * 3; // 9
            case ShaderDataType::Mat4:
                return 4 * 4; // 16

            // --- Signed Integers ---
            case ShaderDataType::Int: return 1;
            case ShaderDataType::Int2: return 2;
            case ShaderDataType::Int3: return 3;
            case ShaderDataType::Int4: return 4;

            // --- Unsigned Integers ---
            case ShaderDataType::UInt: return 1;
            case ShaderDataType::UInt2: return 2;
            case ShaderDataType::UInt3: return 3;
            case ShaderDataType::UInt4: return 4;

            // --- Boolean ---
            case ShaderDataType::Bool: return 1;
        }

        // CZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }
};

class VertexBufferLayout {
private:
    std::vector<BufferElement> m_Elements;
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

    VertexBufferLayout(const std::initializer_list<BufferElement>& elements)
        : m_Elements(elements) {
        CalculateOffsetsAndStride();
    }

    inline uint32 GetStride() const { return m_Stride; }
    inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

    void AddElement(ShaderDataType type, const std::string& name, uint32_t location = 0) {
        m_Elements.push_back({ type, name, 0, location });
        CalculateOffsetsAndStride();
    }

    std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
    std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
};

// clang-format off
enum class PixelFormat {
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

#define POLYGON_MODE_LIST                                                                          \
    X(Fill)                                                                                        \
    X(Line)
// X(Point) // Metal does not support setting VK_POLYGON_MODE_POINT dynamically

#define X(name) name,
enum class PolygonMode { POLYGON_MODE_LIST };
#undef X

#define X(name) #name,
static constexpr std::array<const char*, 2> FPolygonModeStrings = { POLYGON_MODE_LIST };
#undef X

#define CULL_MODE_LIST                                                                             \
    X(None)                                                                                        \
    X(Front)                                                                                       \
    X(Back)                                                                                        \
    X(FrontAndBack)

#define X(name) name,
enum class CullMode { CULL_MODE_LIST };
#undef X

#define X(name) #name,
static constexpr std::array<const char*, 4> FCullModeStrings = { CULL_MODE_LIST };
#undef X

enum class CompareOp : uint8_t {
    Never          = 0,
    Less           = 1,
    Equal          = 2,
    LessOrEqual    = 3,
    Greater        = 4,
    NotEqual       = 5,
    GreaterOrEqual = 6,
    Always         = 7,
};

enum class Filter : uint8_t {
    Nearest = 0,
    Linear  = 1,

    Default = Linear
};

enum class AddressMode : uint8_t {
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
enum class MipmapMode : uint8_t {
    Nearest = 0,
    Linear  = 1,

    Default = Linear
};

enum class ImageUsage : uint32_t {
    Sampled             = 1 << 0, // For shader reading (e.g., texture sampling)
    ColorAttachment     = 1 << 1, // For framebuffer color attachments
    DepthStencil        = 1 << 2, // For framebuffer depth/stencil attachments
    Storage             = 1 << 3, // For Compute Shader read/write (RWTexture)
    TransferSrc         = 1 << 4, // For Copy Source (e.g., Readback or Mip Generation)
    TransferDst         = 1 << 5, // For Copy Destination (e.g., Upload or Mip Generation)
    TransientAttachment = 1 << 6  // For transient resources optimization
};
ENUM_CLASS_FLAGS(ImageUsage);

enum class ImageLayout {
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

enum class ImageViewType {
    View1D = 0,
    View2D,
    View3D,
    ViewCube,
    View1DArray,
    View2DArray,
    ViewCubeArray
};

enum class TextureType { Texture2D, TextureCube };

enum class TextureUsage : uint32_t {
    None       = 0,
    Texture    = 1 << 0, // Regular sampled texture (e.g., Shader Resource like Albedo maps)
    Attachment = 1 << 1, // Rendering target (Color/Depth/Stencil)
    Storage    = 1 << 2, // Compute shader read/write (RWTexture)
    Upload     = 1 << 3, // Needs to be updated from CPU
    Readback   = 1 << 4  // Needs to be read back to CPU
};
ENUM_CLASS_FLAGS(TextureUsage);

enum class PresentMode {
    Immediate = 0, // VSync OFF. Minimal latency, high tearing.
    Mailbox,       // VSync OFF (Fast). No tearing, but heavy GPU load.
    FIFO,          // VSync ON. Standard synchronized presentation.
    FIFO_Relaxed,  // VSync ON (Adaptive). Latency-optimized.

    Unkown
};

/**
 * CommandPoolFlags - Maps to underlying API flags (e.g., VkCommandPoolCreateFlagBits).
 * Defines the memory allocation behavior and reset capabilities of the pool.
 */
enum class CommandPoolFlags : uint32 {
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
ENUM_CLASS_FLAGS(CommandPoolFlags);

// ===== Buffer Usage Flags =====
enum class BufferUsage : uint32 {
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
ENUM_CLASS_FLAGS(BufferUsage);

// ===== Memory Type Flags =====
enum class MemoryType : uint32 {
    Unknown         = 0,
    DeviceLocal     = 1 << 0, // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    HostVisible     = 1 << 1, // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    HostCoherent    = 1 << 2, // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    HostCached      = 1 << 3, // VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    LazilyAllocated = 1 << 4, // VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
    Protected       = 1 << 5, // VK_MEMORY_PROPERTY_PROTECTED_BIT
    DeviceAddress   = 1 << 6, // VK_MEMORY_PROPERTY_DEVICE_ADDRESS_BIT
};
ENUM_CLASS_FLAGS(MemoryType);

} // namespace CZ

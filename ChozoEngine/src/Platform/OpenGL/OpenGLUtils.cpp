#include "OpenGLUtils.h"

#include "Chozo/Debug/Log.h"
#include <glad/glad.h>

// namespace Chozo {

    // static std::string GLenumToString(GLenum enumVal) {
    //     switch (enumVal) {
    //         case GL_RED_INTEGER: return "GL_RED_INTEGER";
    //         case GL_R8I: return "GL_R8I";
    //         case GL_R8UI: return "GL_R8UI";
    //         case GL_R16I: return "GL_R16I";
    //         case GL_R16UI: return "GL_R16UI";
    //         case GL_R32I: return "GL_R32I";
    //         case GL_R32UI: return "GL_R32UI";
    //         case GL_R32F: return "GL_R32F";
    //         case GL_RG8: return "GL_RG8";
    //         case GL_RG16F: return "GL_RG16F";
    //         case GL_RG32F: return "GL_RG32F";
    //         case GL_RGB: return "GL_RGB";
    //         case GL_RGB8: return "GL_RGB8";
    //         case GL_RGB16F: return "GL_RGB16F";
    //         case GL_RGBA: return "GL_RGBA";
    //         case GL_RGBA8: return "GL_RGBA8";
    //         case GL_RGBA16F: return "GL_RGBA16F";
    //         case GL_RGBA32F: return "GL_RGBA32F";
    //         case GL_R11F_G11F_B10F: return "GL_R11F_G11F_B10F";
    //         case GL_SRGB: return "GL_SRGB";
    //         case GL_DEPTH_STENCIL: return "GL_DEPTH_STENCIL";
    //         case GL_DEPTH32F_STENCIL8: return "GL_DEPTH32F_STENCIL8";
    //         case GL_DEPTH24_STENCIL8: return "GL_DEPTH24_STENCIL8";
    //         case GL_INT: return "GL_INT";
    //         case GL_FLOAT: return "GL_FLOAT";
    //         case GL_UNSIGNED_BYTE: return "GL_UNSIGNED_BYTE";
    //         case GL_UNSIGNED_INT_24_8: return "GL_UNSIGNED_INT_24_8";
    //         default: return "Unknown GLenum";
    //     }
    // }

// std::ostream& operator<<(std::ostream& os, const GLenum& enumVal)
// {
//     const char* msg;
//     switch (enumVal)
//     {
//         case GL_R8I:          msg = "GL_R8I"; break;
//         case GL_R8UI:         msg = "GL_R8UI"; break;
//         case GL_R16I:         msg = "GL_R16I"; break;
//         case GL_R16UI:        msg = "GL_R16UI"; break;
//         case GL_R32I:         msg = "GL_R32I"; break;
//         case GL_R32UI:        msg = "GL_R32UI"; break;
//         case GL_R32F:         msg = "GL_R32F"; break;
//         case GL_RG8:          msg = "GL_RG8"; break;
//         case GL_RGBA:         msg = "GL_RGBA"; break;
//         case GL_DEPTH24_STENCIL8: msg = "GL_DEPTH24_STENCIL8"; break;
//         case GL_DEPTH_COMPONENT:  msg = "GL_DEPTH_COMPONENT"; break;
//         default:              msg = "Unknown GLenum"; break;
//     }

//     return os << msg; 
// }

// template <>
// struct fmt::formatter<GLenum> {
//     // Parse format specifications if needed (not used here)
//     constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
//         return ctx.end();
//     }

//     // Format the GLenum as a string using the GLenumToString function
//     template <typename FormatContext>
//     auto format(const GLenum& enumVal, FormatContext& ctx) -> decltype(ctx.out()) {
//         return format_to(ctx.out(), "{}", GLenumToString(enumVal));
//     }
// };

#define EXIT_ON_GL_ERROR
void gl_check_error(const char* function, const char* file, int line)
{
    GLenum error = 0;
    bool is_error = false;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        is_error = true;
        const char* errorMsg;
        switch (error)
        {
            case GL_INVALID_ENUM:                 errorMsg = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                errorMsg = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:            errorMsg = "GL_INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:               errorMsg = "GL_STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:              errorMsg = "GL_STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                errorMsg = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:errorMsg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            case GL_CONTEXT_LOST:                 errorMsg = "GL_CONTEXT_LOST"; break;
            default:                              errorMsg = "Unknown error"; break;
        }

        // Extract just the file name from the full file path (optional)
        const char* fileName = std::strrchr(file, '/');
        fileName = fileName ? fileName + 1 : file;

        // Log the error
        CZ_CORE_ERROR("OpenGL Error: {0} at function {1} ({2}:{3})", errorMsg, function, fileName, line);
    }
#ifdef EXIT_ON_GL_ERROR
    if (is_error)
        exit(1);
#endif
}

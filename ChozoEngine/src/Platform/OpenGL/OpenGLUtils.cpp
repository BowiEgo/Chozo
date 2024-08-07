#include "OpenGLUtils.h"

#include "Chozo/Debug/Log.h"
#include <glad/glad.h>

#define EXIT_ON_GL_ERROR
void gl_check_error(const char* function, const char* file, int line)
{
    GLenum error = 0;
    bool is_error = false;
    while ( (error = glGetError()) != GL_NO_ERROR)
    {
        is_error = true;
        switch (error)
        {
        case GL_INVALID_ENUM: CZ_CORE_ERROR("GL_INVALID_ENUM");
            break;
        case GL_INVALID_VALUE: CZ_CORE_ERROR("GL_INVALID_VALUE");
            break;
        case GL_INVALID_OPERATION: CZ_CORE_ERROR("GL_INVALID_OPERATION");
            break;
        case GL_STACK_OVERFLOW: CZ_CORE_ERROR("GL_STACK_OVERFLOW");
            break;
        case GL_STACK_UNDERFLOW: CZ_CORE_ERROR("GL_STACK_UNDERFLOW");
            break;
        case GL_OUT_OF_MEMORY: CZ_CORE_ERROR("GL_OUT_OF_MEMORY");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: CZ_CORE_ERROR("GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
        case GL_CONTEXT_LOST: CZ_CORE_ERROR("GL_CONTEXT_LOST");
            break;
        default:
            CZ_CORE_ERROR("Unkonwn error code %d", error);
        }
        CZ_CORE_ERROR(" encountered at function '%s' (%s:%d)\n", function, file, line);
    }
#ifdef EXIT_ON_GL_ERROR
    if (is_error)
        exit(1);
#endif
}

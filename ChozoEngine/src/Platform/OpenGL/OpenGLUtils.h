#pragma once

#include <glad/glad.h>
#include "Chozo/Renderer/Image.h"

void gl_check_error(const char* function, const char* file, int line);

#define CHECK_GL_ERRORS

#ifdef CHECK_GL_ERRORS
#define GCE gl_check_error(__FUNCTION__, __FILE__, __LINE__);
#else
#define GCE
#endif

namespace Chozo
{
    GLenum GetGLFormat(const ImageFormat& format);
    GLenum GetGLDataFormat(const ImageFormat& format);
    GLenum GetGLDataType(const ImageFormat& format);
    GLenum GetGLParameter(const ImageParameter& param);
}
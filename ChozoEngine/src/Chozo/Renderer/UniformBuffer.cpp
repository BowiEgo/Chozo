#include "UniformBuffer.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Chozo {

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLUniformBuffer>(size);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}

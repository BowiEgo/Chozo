#include "Pipeline.h"

#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLPipeline.h"

namespace Chozo {

    Ref<Pipeline> Pipeline::Create(PipelineSpecification &spec)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLPipeline>(spec);
        }

        CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}
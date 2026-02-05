#include "RHIShader.h"

// #include "OpenGLRHIShader.h"
// #include "RendererAPI.h"
// #include "VulkanRHIShader.h"

// TRef<IRHIShader> IRHIShader::Create(const FRHIShaderCreateInfo &info) {
//     switch (CRendererAPI::GetType()) {
//     case CRendererAPI::EType::None:
//         CZ_CORE_ASSERT(false, "RendererAPI::None is currently not
//         supported!"); return nullptr;
//     case CRendererAPI::EType::OpenGL:
//         // return TRef<COpenGLRHIShader>(info);
//     case CRendererAPI::EType::Vulkan:
//         return TRef<CVulkanRHIShader>(info);
//     }

//     CZ_CORE_ASSERT(false, "Unknown RendererAPI!");
//     return nullptr;
// }
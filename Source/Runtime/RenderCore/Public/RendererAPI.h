#pragma once

namespace Chozo {

class RENDERCORE_API RendererAPI {
public:
    enum class API { None = 0, Vulkan = 1, OpenGL = 2, DirectX12 = 3 };

    static API GetAPI() { return s_API; }
    static void SetAPI(API api) { s_API = api; }

private:
    inline static API s_API = API::None;
};
} // namespace Chozo
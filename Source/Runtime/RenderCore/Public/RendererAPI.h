#pragma once

class RENDERCORE_API CRendererAPI {
public:
    enum class EType { None = 0, Vulkan = 1, OpenGL = 2, DirectX12 = 3 };

    static EType GetType() { return s_Type; }
    static void SetType(EType api) { s_Type = api; }

private:
    inline static EType s_Type = EType::None;
};
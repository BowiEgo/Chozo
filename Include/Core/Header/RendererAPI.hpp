#pragma once

namespace CZ {

class RendererAPI {
public:
    enum class Type { None = 0, Vulkan = 1, OpenGL = 2, DirectX12 = 3 };

    static Type GetType() { return s_Type; }
    static void SetType(Type api) { s_Type = api; }

private:
    inline static Type s_Type = Type::None;
};
} // namespace CZ

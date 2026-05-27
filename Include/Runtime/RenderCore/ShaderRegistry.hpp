#pragma once

#include <Runtime/RenderCore/AssetRegistry.hpp>
#include <Runtime/RenderCore/Shader.hpp>

namespace CZ {

struct ShaderObj;

template <> struct ResourceLoaderTraits<ShaderObj> {
    static Scope<ShaderObj> Load(const std::string& virtualPath);
};

using ShaderRegistry = AssetRegistry<ShaderObj>;
} // namespace CZ
#pragma once

#include <Runtime/RenderCore/AssetRegistry.hpp>
#include <Runtime/RenderCore/Mesh.hpp>

namespace CZ {

struct MeshObj;

template <> struct ResourceLoaderTraits<MeshObj> {
    static Scope<MeshObj> Load(const std::string& virtualPath);
};

using MeshRegistry = AssetRegistry<MeshObj>;
} // namespace CZ
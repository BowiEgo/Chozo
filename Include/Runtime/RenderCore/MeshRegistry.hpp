#pragma once

#include <Runtime/RenderCore/AssetRegistry.hpp>
#include <Runtime/RenderCore/Mesh.hpp>
#include <Runtime/RenderCore/MeshParams.hpp>

namespace CZ {

struct MeshObj;

template <> struct ResourceLoaderTraits<MeshObj> {
    static Scope<MeshObj> Load(const std::string& virtualPath);

    static Scope<MeshObj> Create(const MeshParams& params);
};

using MeshRegistry = AssetRegistry<MeshObj>;
} // namespace CZ
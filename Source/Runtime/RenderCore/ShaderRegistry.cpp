#include <Runtime/RenderCore/Shader.hpp>
#include <Runtime/RenderCore/ShaderCompiler.hpp>
#include <Runtime/RenderCore/ShaderRegistry.hpp>

namespace CZ {

Scope<ShaderObj> ResourceLoaderTraits<ShaderObj>::Load(const std::string& virtualPath) {
    auto realPath = VFS::Resolve(virtualPath);
    auto name     = realPath.stem();

    ShaderSpecification spec{ name, virtualPath };

    ShaderCompilerMultiInput input;
    input.VirtualPath = virtualPath;

    std::unordered_map<ShaderStage, ShaderCompilerOutput> outputs;

    if (ShaderCompiler::Get().Compile(input, outputs)) {
        return CZ_CREATE_SCOPE(MEMORY_USAGE_ASSET, ShaderObj, spec, outputs);
    }
    return nullptr;
}

template <> void AssetRegistry<ShaderObj>::Init() {}

template <> void AssetRegistry<ShaderObj>::Shutdown() {}

} // namespace CZ
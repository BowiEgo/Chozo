#include "AssetManager.h"

#include "FileSystemUtils.h"
#include "ShaderUtils.h"
#include "TextureImporter.h"

#include "RHIAPI.h"

#include "PBRMaterialParams.h"

DEFINE_LOG_CATEGORY(LogAssetManager);

CAssetManager::CAssetManager() {
    m_ShaderCompiler    = CreateScope<CShaderCompiler>();
    m_CheckboardTexture = GetOrLoadTexture("textures://CheckerboardTexture.png");
}

void CAssetManager::ClearCaches() {
    m_Caches.clear();
    m_TextureCaches.clear();
}

void CAssetManager::Shutdown() {
    ClearCaches();
    m_CheckboardTexture.Reset();
}

TRef<CTexture> CAssetManager::GetOrLoadTexture(const std::string& virtualPath) {
    std::filesystem::path path = VFS::Resolve(virtualPath);
    std::string pathString     = path.string();

    auto it = m_TextureCaches.find(pathString);
    if (it != m_TextureCaches.end()) {
        return it->second;
    }

    FImageFormatDesc desc;
    uint32_t w, h;
    auto imageData = FTextureImporter::ToBufferFromFile(pathString, desc, w, h);
    CZ_LOG(LogAssetManager, Trace, "Import Texture Buffer: {}, {}", w, h);

    std::string ext = path.extension().string();
    bool isHDR      = (ext == ".hdr" || ext == ".HDR");

    FTextureSpecification spec;
    spec.Name        = "Texture";
    spec.Size        = { w, h };
    spec.Format      = ChozoUtils::FileSystem::PixelFormatFromDesc(desc);
    spec.Usage       = ETextureUsage::Texture;
    spec.SamplerSpec = FSamplerSpecification::Repeat();
    if (isHDR) {
        // spec.Type = ETextureType::TextureCube;
    } else {
        spec.Type = ETextureType::Texture2D;
    }

    TRef<CTexture> asset = CreateRef<CTexture>(spec, imageData);
    FAssetHandle handle  = FAssetHandle::Generate();

    asset->SetHandle(handle);
    m_TextureCaches[pathString] = asset;
    m_Caches[handle]            = asset;

    return asset;
}

TRef<CShader> CAssetManager::GetOrLoadShader(const FShaderSpecification& spec) {
    // CZ_LOG(LogAssetManager, Trace, "Loading Shader: {}", spec.Name);

    // auto it = m_Caches.find(specs);
    // if (it != m_Caches.end()) {
    //     return it->second;
    // }

    CZ_LOG(LogAssetManager, Trace, "Compiling Shader: {}", spec.Name);

    FShaderCompilerMultiInput input;
    input.VirtualPath = spec.VirtualPath;
    input.Stages      = spec.Stages;
    input.Macros.Add(spec.Definitions);

    std::unordered_map<EShaderStage, FShaderCompilerOutput> outputs;

    if (bool success = m_ShaderCompiler->Compile(input, outputs)) {
        TRef<CShader> asset = CreateRef<CShader>(spec, outputs);
        FAssetHandle handle = FAssetHandle::Generate();

        asset->SetHandle(handle);
        m_Caches[handle] = asset;

        return asset;
    }

    return nullptr;
}

TRef<CMaterial> CAssetManager::GetOrLoadMaterial(const FMaterialSpecification& spec) {
    // CZ_LOG(LogAssetManager, Trace, "Loading Material: {}", spec.Name);

    // auto it = m_Caches.find(specs);
    // if (it != m_Caches.end()) {
    //     return it->second;
    // }

    TRef<CMaterial> pbrMaterial = CreateRef<CMaterial>(
        spec, FMaterialProps{ FPBRMaterialParams{
                  { 1.0f, 0.0f, 0.0f, 1.0f }, 0.5f, 0.5f, 1.0f, 1.0f, false, false, false } });

    FAssetHandle handle = FAssetHandle::Generate();
    pbrMaterial->SetHandle(handle);
    m_Caches[handle] = pbrMaterial;

    return pbrMaterial;
}

#pragma once

#include "MeshParams.h"
#include "ProceduralMesh.h"
#include "Ref.h"
#include "Scope.h"
#include "TypeRegister.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMeshRegistry, Info);

struct IMeshGenerator {
    virtual ~IMeshGenerator()                                       = default;
    virtual TRef<FProceduralMesh> CreateMesh(const IParams& params) = 0;
};

template <typename ParamsType, typename MeshType> class TMeshGenerator : public IMeshGenerator {
public:
    TRef<FProceduralMesh> CreateMesh(const IParams& params) override {
        const auto& typedParams = static_cast<const ParamsType&>(params);
        return CreateRef<MeshType>(typedParams);
    }
};

class RENDER_CORE_API FMeshRegister {
public:
    static FMeshRegister& Get();
    static void Init();

    FMeshRegister(const FMeshRegister&)            = delete;
    FMeshRegister& operator=(const FMeshRegister&) = delete;

    FTypeInfo RegisterMeshType(const std::string& name, bool bBuiltin = false) {
        FTypeInfo typeInfo =
            FTypeRegister::Get().RegisterType("Mesh_" + name, bBuiltin, ETypeCategory::Mesh);

        m_Types.push_back(typeInfo);
        return typeInfo;
    }

    template <typename T> void RegisterParamsType(bool bBuiltin) {
        std::string typeName  = T::GetStaticTypeName();
        m_Factories[typeName] = CreateScope<TParamsFactory<T>>();

        RegisterMeshType(typeName, bBuiltin);
    }

    template <typename ParamsType, typename MeshType>
    void RegisterMeshGenerator(const std::string& typeName) {
        m_Generators[typeName] = CreateScope<TMeshGenerator<ParamsType, MeshType>>();
    }

    template <typename ParamsType, typename MeshType>
    void RegisterMeshType(const std::string& typeName, bool bBuiltin = false) {
        RegisterParamsType<ParamsType>(bBuiltin);
        RegisterMeshGenerator<ParamsType, MeshType>(typeName);
    }

    FMeshParams CreateParams(const std::string& typeName) const {
        auto it = m_Factories.find(typeName);
        if (it != m_Factories.end()) {
            auto params = it->second->CreateDefault();
            if (params) {
                return FMeshParams(std::move(params));
            }
        }
        return FMeshParams();
    }

    TRef<FProceduralMesh> CreateMesh(const IParams& params) {
        auto it = m_Generators.find(params.GetTypeName());
        if (it != m_Generators.end()) {
            return it->second->CreateMesh(params);
        }

        CZ_LOG(LogMeshRegistry, Warning, "No generator found for type: {}", params.GetTypeName());
        return nullptr;
    }

    FMeshBuffer* CacheStaticData(const IParams& params, const FMeshBuffer& data) {
        size_t hash          = params.GetHash();
        std::string typeName = params.GetTypeName();
        std::string key      = typeName + "_" + std::to_string(hash);

        m_StaticDataCache[key] = std::move(data);
        return &m_StaticDataCache[key];
    }

    FMeshBuffer* GetStaticCachedData(const IParams& params) {
        size_t hash          = params.GetHash();
        std::string typeName = params.GetTypeName();
        std::string key      = typeName + "_" + std::to_string(hash);

        auto it = m_StaticDataCache.find(key);
        if (it != m_StaticDataCache.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void ClearCache() { m_StaticDataCache.clear(); }

    const std::vector<FTypeInfo>& GetAllTypes() const { return m_Types; }

    bool IsParamsTypeRegistered(const std::string& typeName) const {
        return m_Factories.find(typeName) != m_Factories.end();
    }

    bool IsMeshGeneratorRegistered(const std::string& typeName) const {
        return m_Generators.find(typeName) != m_Generators.end();
    }

private:
    FMeshRegister() {
        RegisterMeshType("Regular", true);
        // Built-in types are registered externally to avoid circular dependencies
        // Call RegisterMeshType in MeshRegistry.cpp
    }
    ~FMeshRegister() = default;

    std::unordered_map<std::string, TScope<IParamsFactory>> m_Factories;
    std::unordered_map<std::string, TScope<IMeshGenerator>> m_Generators;
    std::unordered_map<std::string, FMeshBuffer> m_StaticDataCache;
    std::vector<FTypeInfo> m_Types;
};

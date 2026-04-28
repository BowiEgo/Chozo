#pragma once

#include "Material.h"
#include "MaterialParamsWrapper.h"
#include "Ref.h"
#include "Scope.h"
#include "TypeRegister.h"

#include "RenderCoreExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMaterialRegistry, Info);

struct IMaterialGenerator {
    virtual ~IMaterialGenerator()                                         = default;
    virtual TRef<CMaterial> CreateMaterial(const IMaterialParams& params) = 0;
};

template <typename ParamsType, typename MaterialType>
class TMaterialGenerator : public IMaterialGenerator {
public:
    TRef<CMaterial> CreateMaterial(const IMaterialParams& params) override {
        const auto& typedParams = static_cast<const ParamsType&>(params);
        return CreateRef<MaterialType>(typedParams);
    }
};

class RENDER_CORE_API FMaterialRegister {
public:
    static FMaterialRegister& Get();
    static void Init();

    FMaterialRegister(const FMaterialRegister&)            = delete;
    FMaterialRegister& operator=(const FMaterialRegister&) = delete;

    FTypeInfo RegisterMaterialType(const std::string& name, bool bBuiltin = false) {
        FTypeInfo typeInfo = FTypeRegister::Get().RegisterType("Material_" + name, bBuiltin,
                                                               ETypeCategory::Material);

        m_Types.push_back(typeInfo);
        return typeInfo;
    }

    template <typename T> void RegisterParamsType(bool bBuiltin) {
        std::string typeName  = T::GetStaticTypeName();
        m_Factories[typeName] = CreateScope<TParamsFactory<T>>();

        RegisterMaterialType(typeName, bBuiltin);
    }

    template <typename ParamsType, typename MaterialType>
    void RegisterMaterialGenerator(const std::string& typeName) {
        m_Generators[typeName] = CreateScope<TMaterialGenerator<ParamsType, MaterialType>>();
    }

    template <typename ParamsType, typename MaterialType>
    void RegisterMaterialType(const std::string& typeName, bool bBuiltin = false) {
        RegisterParamsType<ParamsType>(bBuiltin);
        RegisterMaterialGenerator<ParamsType, MaterialType>(typeName);
    }

    FMaterialParamsWrapper CreateParams(const std::string& typeName) const {
        auto it = m_Factories.find(typeName);
        if (it != m_Factories.end()) {
            auto params = it->second->CreateDefault();
            if (params) {
                return FMaterialParamsWrapper(std::move(params));
            }
        }
        return FMaterialParamsWrapper();
    }

    TRef<CMaterial> CreateMaterial(const IMaterialParams& params) {
        auto it = m_Generators.find(params.GetTypeName());
        if (it != m_Generators.end()) {
            return it->second->CreateMaterial(params);
        }

        CZ_LOG(LogMaterialRegistry, Warning, "No generator found for type: {}",
               params.GetTypeName());
        return nullptr;
    }

    FMaterialBuffer* CacheStaticData(const IMaterialParams& params, const FMaterialBuffer& data) {
        size_t hash          = params.GetHash();
        std::string typeName = params.GetTypeName();
        std::string key      = typeName + "_" + std::to_string(hash);

        m_StaticDataCache[key] = std::move(data);
        return &m_StaticDataCache[key];
    }

    FMaterialBuffer* GetStaticCachedData(const IMaterialParams& params) {
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

    bool IsMaterialGeneratorRegistered(const std::string& typeName) const {
        return m_Generators.find(typeName) != m_Generators.end();
    }

private:
    FMaterialRegister() {
        RegisterMaterialType("Regular", true);
        // Built-in types are registered externally to avoid circular dependencies
        // Call RegisterMaterialType in MaterialRegistry.cpp
    }
    ~FMaterialRegister() = default;

    std::unordered_map<std::string, TScope<IMaterialParamsFactory>> m_Factories;
    std::unordered_map<std::string, TScope<IMaterialGenerator>> m_Generators;
    std::vector<FTypeInfo> m_Types;
};

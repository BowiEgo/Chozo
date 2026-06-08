#pragma once

#include <Runtime/RHI/GraphicsBuffer.hpp>
#include <Runtime/RHI/Pipeline.hpp>
#include <Runtime/RenderCore/Shader.hpp>

namespace CZ {

struct MaterialSpecification {
    std::string Name;
    Shader Shader;
    std::vector<PixelFormat> ColorFormats;
    PixelFormat DepthFormat = PixelFormat::D32_SFLOAT;
};

class MaterialObj {
public:
    MaterialObj(const MaterialSpecification& spec, const MaterialParamsWrapper& params)
        : m_Spec(spec), m_Params(params) {};
    virtual ~MaterialObj() = default;

    const MaterialSpecification GetSpec() const { return m_Spec; }
    Shader GetShader() const { return m_Spec.Shader; }

    MaterialParamsWrapper& GetParams() { return m_Params; }
    const MaterialParamsWrapper& GetParams() const { return m_Params; }
    const Pipeline GetPipeline();

    void MarkDirty() { m_bIsDirty = true; }
    void CreateDescriptorSet();

private:
    MaterialSpecification m_Spec;
    MaterialParamsWrapper m_Params;

    Pipeline m_Pipeline;
    GraphicsBuffer m_UniformBuffer;

    bool m_bIsDirty = true;
    DescriptorSet m_DescriptorSet;
};

class Material : public Asset<class MaterialObj> {
public:
    AssetType GetType() const override { return AssetType::Material; }

    const std::string GetName() const override { return (*this)->GetSpec().Name; }
};

} // namespace CZ
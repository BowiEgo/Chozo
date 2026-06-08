#include <Runtime/RHI/RHIAPI.hpp>
#include <Runtime/RenderCore/Material.hpp>

namespace CZ {

const Pipeline MaterialObj::GetPipeline() {
    if (m_Pipeline) return m_Pipeline;

    auto params = m_Params.Get();

    PipelineSpecification spec;
    spec.Name               = m_Spec.Name + "_Pipeline";
    spec.SetLayouts         = m_Spec.Shader->GetAllSetLayouts();
    spec.VertexLayout       = m_Spec.Shader->GetVertexLayout();
    spec.PushConstantRanges = m_Spec.Shader->GetPushConstantRanges();
    spec.ColorFormats       = m_Spec.ColorFormats;
    spec.DepthFormat        = m_Spec.DepthFormat;
    spec.PolygonMode        = params->PolygonMode;
    spec.CullMode           = params->CullMode;
    // spec.bDepthTestEnable   = params->bDepthTestEnable;
    // spec.bDepthWriteEnable  = params->bDepthWriteEnable;
    // spec.DepthCompareOp     = params->DepthCompareOp;

    m_Pipeline = RHIAPI::Get()->GetGraphicsContext()->GetDevice()->CreatePipeline(
        spec, m_Spec.Shader->GetShaderResources(), m_Spec.Shader->GetReflection());

    return m_Pipeline;
}

void MaterialObj::CreateDescriptorSet() {
    auto device = RHIAPI::Get()->GetGraphicsContext()->GetDevice();
    if (!device) return;

    auto setLayout = m_Spec.Shader->GetSetLayout(1);
    if (!setLayout) {
        CZ_LOG(LogMaterial, Error, "Material '{}' has no set layout for set 1",
               m_Spec.Name.c_str());
        return;
    }

    std::vector<DescriptorBinding> bindings;

    for (auto shaderRes : m_Spec.Shader->GetShaderResources()) {
        auto reflection = shaderRes->GetReflection();
        for (const auto& uniform : reflection.Uniforms) {
            if (uniform.Set != 1) continue;

            DescriptorBinding binding;
            binding.Binding     = uniform.Binding;
            binding.Type        = uniform.Type;
            binding.ImageLayout = ImageLayout::ShaderReadOnlyOptimal;

            if (binding.Type == UniformType::CombinedImageSampler) {
                std::any value = m_Params.Get()->GetParamValue(
                    ChozoUtils::String::RemovePrefix(uniform.Name, "u_"));

                AssetHandle handle = std::any_cast<AssetHandle>(value);
                auto texture       = AssetManager::Get().GetAsset(handle).As<CTexture>();
                if (texture && texture->GetResource()) {
                    binding.Resource = texture->GetResource()->GetImage();
                    binding.Sampler  = texture->GetResource()->GetSampler().get();
                } else {
                    CZ_LOG(LogMaterial, Warning, "Material '{}' missing texture for binding '{}'",
                           m_Spec.Name, uniform.Name);
                    auto texture     = AssetManager::Get().GetCheckboardTexture();
                    binding.Resource = texture->GetResource()->GetImage();
                    binding.Sampler  = texture->GetResource()->GetSampler().get();
                }
            } else if (binding.Type == UniformType::UniformBuffer && uniform.Name == "u_Material") {

                Vector4 baseColor =
                    std::any_cast<Vector4>(m_Params.Get()->GetParamValue("BaseColor"));
                float metallic  = std::any_cast<float>(m_Params.Get()->GetParamValue("Metallic"));
                float roughness = std::any_cast<float>(m_Params.Get()->GetParamValue("Roughness"));
                float normalStrength =
                    std::any_cast<float>(m_Params.Get()->GetParamValue("NormalStrength"));
                float emissiveStrength =
                    std::any_cast<float>(m_Params.Get()->GetParamValue("EmissiveStrength"));
                bool useAlbedoMap =
                    std::any_cast<bool>(m_Params.Get()->GetParamValue("UseAlbedoMap"));
                bool useNormalMap =
                    std::any_cast<bool>(m_Params.Get()->GetParamValue("UseNormalMap"));
                bool useRMAOMap = std::any_cast<bool>(m_Params.Get()->GetParamValue("UseRMAOMap"));

                struct alignas(16) MaterialUniforms {
                    Vector4 BaseColor;
                    float Metallic;
                    float Roughness;
                    float NormalStrength;
                    float EmissiveStrength;
                    int UseAlbedoMap;
                    int UseNormalMap;
                    int UseRMAOMap;
                    char padding[4];
                };

                MaterialUniforms matUniforms;
                matUniforms.BaseColor        = baseColor;
                matUniforms.Metallic         = metallic;
                matUniforms.Roughness        = roughness;
                matUniforms.NormalStrength   = normalStrength;
                matUniforms.EmissiveStrength = emissiveStrength;
                matUniforms.UseAlbedoMap     = useAlbedoMap ? 1 : 0;
                matUniforms.UseNormalMap     = useNormalMap ? 1 : 0;
                matUniforms.UseRMAOMap       = useRMAOMap ? 1 : 0;

                BufferSpecification bufferSpec;
                bufferSpec.Usage      = BufferUsage::UniformBuffer;
                bufferSpec.Size       = sizeof(MaterialUniforms);
                bufferSpec.MemoryType = MemoryType::HostVisible | MemoryType::HostCoherent;
                bufferSpec.Name       = m_Spec.Name + "_MaterialUniformBuffer";

                Buffer data(&matUniforms, bufferSpec.Size);

                if (m_UniformBuffer) {
                    m_UniformBuffer->SetData(data);
                } else {
                    m_UniformBuffer = IRHIAPI::CreateBuffer(bufferSpec, data);
                }
                binding.Resource = m_UniformBuffer.get();
            }

            bindings.push_back(binding);
        }
    }

    m_DescriptorSet = device->GetOrCreateDescriptorSet(setLayout, bindings);
    m_bIsDirty      = false;
}

void MaterialObj::Bind(IRHICommandList* cmdList) {
    if (m_bIsDirty) CreateDescriptorSet();

    cmdList->BindPipeline(GetPipeline());
    cmdList->SetPolygonMode(m_Params.Get()->PolygonMode);
    // cmdList->SetCullMode(m_Params.Get()->CullMode);

    if (m_DescriptorSet) {
        cmdList->BindDescriptorSets(1, m_DescriptorSet);
    }

    // 4. 如果材质需要全局的 Push Constants（例如每物体的模型矩阵则不应在此，而是每物体绑定）。
    //    如果需要材质内置的 Push Constants（例如材质常量），可在此处设置。
    //    例如：
    //    struct MaterialPushConsts { float opacity; };
    //    if (m_PushConstantSize > 0) {
    //        cmdList->PushConstants(&m_PushConstantData, m_PushConstantSize, 0);
    //    }
}

} // namespace CZ
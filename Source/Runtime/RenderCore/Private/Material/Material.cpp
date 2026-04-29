#include "Material.h"

#include "AssetManager.h"
#include "RHIAPI.h"
#include "StringUtils.h"

const TRef<IRHIPipeline> CMaterial::GetPipeline() {
    if (m_Pipeline) return m_Pipeline;

    auto params = m_Params.Get();

    FPipelineSpecification spec;
    spec.Name               = GetName() + "_Pipeline";
    spec.RHIShaders         = m_Spec.Shader->GetShaderResources();
    spec.RHISeyLayouts      = m_Spec.Shader->GetAllSetLayouts();
    spec.VertexLayout       = m_Spec.Shader->GetVertexLayout();
    spec.PushConstantRanges = m_Spec.Shader->GetPushConstantRanges();
    spec.OutputColorFormats = m_Spec.ColorFormats;
    spec.DepthFormat        = m_Spec.DepthFormat;
    spec.PolygonMode        = params->PolygonMode;
    spec.CullMode           = params->CullMode;
    // spec.bDepthTestEnable   = params->bDepthTestEnable;
    // spec.bDepthWriteEnable  = params->bDepthWriteEnable;
    // spec.DepthCompareOp     = params->DepthCompareOp;

    m_Pipeline = IRHIAPI::CreatePipeline(spec);

    return m_Pipeline;
}

void CMaterial::CreateDescriptorSet() {
    auto device = IRHIAPI::GetContext()->GetDevice();
    if (!device) return;

    auto rhiShaders = m_Spec.Shader->GetShaderResources();
    auto setLayout  = m_Spec.Shader->GetSetLayout(1);
    if (!setLayout) {
        CZ_LOG(LogMaterial, Error, "Material '{}' has no set layout for set 1", GetName().c_str());
        return;
    }

    std::vector<FDescriptorBinding> bindings;

    for (auto RHIShader : rhiShaders) {
        auto reflection = RHIShader->GetReflection();
        for (const auto& uniform : reflection.Uniforms) {
            if (uniform.Set != 1) continue;

            FDescriptorBinding binding;
            binding.Binding     = uniform.Binding;
            binding.Type        = uniform.Type;
            binding.ImageLayout = EImageLayout::ShaderReadOnlyOptimal;

            if (binding.Type == EUniformType::CombinedImageSampler) {
                std::any value = m_Params.Get()->GetParamValue(
                    ChozoUtils::String::RemovePrefix(uniform.Name, "u_"));

                FAssetHandle handle = std::any_cast<FAssetHandle>(value);
                auto texture        = CAssetManager::Get().GetAsset(handle).As<CTexture>();
                if (texture && texture->GetResource()) {
                    binding.Resource = texture->GetResource()->GetImage();
                    binding.Sampler  = texture->GetResource()->GetSampler().get();
                } else {
                    CZ_LOG(LogMaterial, Warning, "Material '{}' missing texture for binding '{}'",
                           GetName(), uniform.Name);
                    auto texture     = CAssetManager::Get().GetCheckboardTexture();
                    binding.Resource = texture->GetResource()->GetImage();
                    binding.Sampler  = texture->GetResource()->GetSampler().get();
                }
            } else if (binding.Type == EUniformType::UniformBuffer &&
                       uniform.Name == "u_Material") {

                FVector4 baseColor =
                    std::any_cast<FVector4>(m_Params.Get()->GetParamValue("BaseColor"));
                float metallic  = std::any_cast<float>(m_Params.Get()->GetParamValue("Metallic"));
                float roughness = std::any_cast<float>(m_Params.Get()->GetParamValue("Roughness"));
                float aoIntensity =
                    std::any_cast<float>(m_Params.Get()->GetParamValue("AOIntensity"));
                FVector3 emissiveColor =
                    std::any_cast<FVector3>(m_Params.Get()->GetParamValue("EmissiveColor"));
                float emissiveIntensity =
                    std::any_cast<float>(m_Params.Get()->GetParamValue("EmissiveIntensity"));
                bool useAlbedoMap =
                    std::any_cast<bool>(m_Params.Get()->GetParamValue("UseAlbedoMap"));
                bool useNormalMap =
                    std::any_cast<bool>(m_Params.Get()->GetParamValue("UseNormalMap"));
                bool useRMAOMap = std::any_cast<bool>(m_Params.Get()->GetParamValue("UseRMAOMap"));
                bool useEmissiveMap =
                    std::any_cast<bool>(m_Params.Get()->GetParamValue("UseEmissiveMap"));

                struct alignas(16) MaterialUniforms {
                    FVector4 BaseColor;
                    FVector4 Emissive;
                    float Metallic;
                    float Roughness;
                    float AOIntensity;
                    int UseAlbedoMap;
                    int UseNormalMap;
                    int UseRMAOMap;
                    int UseEmissiveMap;
                    char padding[4];
                };

                MaterialUniforms matUniforms;
                matUniforms.BaseColor      = baseColor;
                matUniforms.Emissive       = FVector4(emissiveColor, emissiveIntensity);
                matUniforms.Metallic       = metallic;
                matUniforms.Roughness      = roughness;
                matUniforms.AOIntensity    = aoIntensity;
                matUniforms.UseAlbedoMap   = useAlbedoMap ? 1 : 0;
                matUniforms.UseNormalMap   = useNormalMap ? 1 : 0;
                matUniforms.UseRMAOMap     = useRMAOMap ? 1 : 0;
                matUniforms.UseEmissiveMap = useEmissiveMap ? 1 : 0;

                FBufferSpecification bufferSpec;
                bufferSpec.Usage      = EBufferUsage::UniformBuffer;
                bufferSpec.Size       = sizeof(MaterialUniforms);
                bufferSpec.MemoryType = EMemoryType::HostVisible | EMemoryType::HostCoherent;
                bufferSpec.Name       = GetName() + "_MaterialUniformBuffer";

                FBuffer data(&matUniforms, bufferSpec.Size);

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

void CMaterial::Bind(IRHICommandList* cmdList) {
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
#pragma once

#include "ImagePool.h"
#include "RHIAPI.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RHITexture.h"
#include "Texture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderGraph, Info);

struct FRDGTexture {
    std::string Name;
    FTextureSpecification Spec;
    IRHIImage* Image     = nullptr;
    IRHITexture* Texture = nullptr;
    bool bExternal = false; // Whether this texture is externally provided (e.g., swapchain image,
    // skybox texture) and shouldn't be pooled

    EImageLayout InitialLayout = EImageLayout::Undefined;
    EImageLayout FinalLayout   = EImageLayout::Undefined;
    EImageLayout CurrentLayout = EImageLayout::Undefined;

    uint32_t FirstPass = 0;
    uint32_t LastPass  = 0;
};

enum class ERenderPassLoadOp { Clear, Load };

class CRDGContext;

struct FRDGPass {
    std::string Name;
    TRef<IRHIPipeline> Pipeline;
    std::vector<FRDGTexture*> Inputs;
    std::vector<FRDGTexture*> Outputs;
    ERenderPassLoadOp LoadOp;
    std::function<void(CRDGContext& ctx)> ExecuteFunc;
};

class CRDGContext {
public:
    CRDGContext(TRef<IRHICommandList> cmd, FRDGPass* pass, int faceIndex = 0, int mip = 0)
        : m_Cmd(cmd), m_Pass(pass), m_FaceIndex(faceIndex), m_Mip(mip) {
        for (auto* input : m_Pass->Inputs) {
            m_Resources[input] = input->Texture;
        }
        for (auto* output : m_Pass->Outputs) {
            m_Resources[output] = output->Texture;
        }
    }

    TRef<IRHICommandList> GetCommandBuffer() const { return m_Cmd; }

    IRHITexture* GetTexture(FRDGTexture* handle) { return m_Resources[handle]; }

    const std::string& GetPassName() const { return m_Pass->Name; }
    const int GetFaceIndex() const { return m_FaceIndex; }
    const int GetMip() const { return m_Mip; }

private:
    TRef<IRHICommandList> m_Cmd;
    FRDGPass* m_Pass;
    int m_FaceIndex, m_Mip;

    std::unordered_map<FRDGTexture*, IRHITexture*> m_Resources;
};

class CRenderGraph {
public:
    CRenderGraph(IRHIContext* ctx) : m_Context(ctx) {}
    ~CRenderGraph();

    // Declare Logical Textures (no physical allocation yet)
    FRDGTexture* CreateRDGTexture(std::string name, const FTextureSpecification& spec) {
        TScope<IRHITexture> texRes = IRHIAPI::CreateTexture(spec, nullptr);
        auto RDGtex                = new FRDGTexture{ name, spec, nullptr, texRes.get() };

        m_InternalResources.push_back(std::move(texRes));
        m_Textures.push_back(RDGtex);
        return RDGtex;
    }

    FRDGTexture* ImportExternalRDGTexture(std::string name, IRHITexture* rhiTex,
                                          const EImageLayout initialLayout,
                                          const EImageLayout finalLayout) {
        for (auto* existingTex : m_Textures) {
            if (existingTex->bExternal && existingTex->Spec == rhiTex->GetSpec()) {
                existingTex->FinalLayout = finalLayout;
                return existingTex;
            }
        }

        // CZ_LOG(LogRenderGraph, Trace, "ImportExternalRDGTexture");

        auto RDGtex = new FRDGTexture{ name, rhiTex->GetSpec(), rhiTex->GetImage(), rhiTex,
                                       true, initialLayout,     finalLayout,        initialLayout };
        m_Textures.push_back(RDGtex);
        return RDGtex;
    }

    // Register Passes with their input/output logical textures and execution logic
    void AddPass(std::string name, TRef<IRHIPipeline> pipeline, std::vector<FRDGTexture*> inputs,
                 std::vector<FRDGTexture*> outputs, ERenderPassLoadOp loadOp,
                 std::function<void(CRDGContext& ctx)> execute) {
        auto pass = new FRDGPass{ name, pipeline, inputs, outputs, loadOp, execute };
        m_Passes.push_back(pass);

        // Update texture lifetime range
        uint32_t passIndex = m_Passes.size() - 1;

        for (auto* t : inputs) {
            t->FirstPass = std::min(t->FirstPass, passIndex);
            t->LastPass  = std::max(t->LastPass, passIndex);
        }

        for (auto* t : outputs) {
            t->FirstPass = std::min(t->FirstPass, passIndex);
            t->LastPass  = std::max(t->LastPass, passIndex);
        }
    }

    // The most critical step, for physical allocation and barrier insertion
    void Compile() {
        std::vector<FRDGTexture*> sortedTextures = m_Textures;
        std::sort(sortedTextures.begin(), sortedTextures.end(),
                  [](auto* a, auto* b) { return a->FirstPass < b->FirstPass; });

        std::vector<std::pair<IRHIImage*, uint32_t>> busyImages;

        for (auto* tex : sortedTextures) {
            if (tex->bExternal) continue;

            uint32_t currentPass = tex->FirstPass;

            for (auto it = busyImages.begin(); it != busyImages.end();) {
                if (currentPass > it->second) {
                    m_Context->GetDevice()->ReturnImageToPool(it->first);
                    it = busyImages.erase(it);
                } else {
                    ++it;
                }
            }

            FImageSpecification physSpec = tex->Spec.ToImageSpec();
            tex->Image =
                m_Context->GetDevice()->GetImageFromPool(physSpec, m_Context->GetCurrentFrame());
            busyImages.push_back({ tex->Image, tex->LastPass });
            tex->Texture->BorrowImage(tex->Image);
        }
    }

    // The actual Vulkan calls happen here
    void Execute(TRef<IRHICommandList> cmd) {
        for (uint32_t i = 0; i < m_Passes.size(); ++i) {

            auto* pass = m_Passes[i];
            std::vector<IRHITexture*> renderTargets;
            // --- Automatic synchronization logic ---
            for (auto* input : pass->Inputs) {
                // Automatically insert barriers: transition from current state to ShaderReadOnly
                IRHIAPI::TransitionImageLayout(cmd, input->Image,
                                               EImageLayout::ShaderReadOnlyOptimal);
            }
            for (auto* output : pass->Outputs) {
                EImageLayout targetLayout;
                auto imageSpec = output->Spec.ToImageSpec();

                if (ChozoUtils::RHI::IsDepthFormat(output->Spec.Format)) {
                    targetLayout = EImageLayout::DepthStencilAttachmentOptimal;
                } else {
                    targetLayout = EImageLayout::ColorAttachmentOptimal;
                }

                IRHIAPI::TransitionImageLayout(cmd, output->Image, targetLayout, 0,
                                               imageSpec.Layers);

                renderTargets.push_back(output->Texture);
            }

            // Execute user-defined rendering logic for this pass
            auto tarSize = renderTargets.size();
            m_Context->SetRenderTargets(renderTargets);
            if (pass->Pipeline) cmd->BindPipeline(pass->Pipeline);
            bool shouldClear = (pass->LoadOp == ERenderPassLoadOp::Clear);

            if (renderTargets[0]->GetSpec().Type == ETextureType::TextureCube) {
                uint32_t mipLevels = renderTargets[0]->GetImage()->GetSpec().MipLevels;
                for (uint32_t mip = 0; mip < mipLevels; ++mip) {
                    for (uint32_t face = 0; face < 6; ++face) {
                        IRHIAPI::BeginRendering(cmd, shouldClear, face);
                        CRDGContext execCtx(cmd, pass, face, mip);
                        pass->ExecuteFunc(execCtx);
                        IRHIAPI::EndRendering(cmd);
                    }
                }
            } else {
                IRHIAPI::BeginRendering(cmd, shouldClear);
                CRDGContext execCtx(cmd, pass);
                pass->ExecuteFunc(execCtx);
                IRHIAPI::EndRendering(cmd);
            }

            std::vector<FRDGTexture*> allRes = pass->Inputs;
            allRes.insert(allRes.end(), pass->Outputs.begin(), pass->Outputs.end());

            for (auto* res : allRes) {
                if (res->LastPass == i && res->FinalLayout != EImageLayout::Undefined) {
                    if (res->CurrentLayout != res->FinalLayout) {
                        IRHIAPI::TransitionImageLayout(cmd, res->Image, res->FinalLayout);
                        res->CurrentLayout = res->FinalLayout;
                    }
                }
            }
        }
    }

private:
    IRHIContext* m_Context;

    std::vector<TScope<IRHITexture>> m_InternalResources;
    std::vector<FRDGTexture*> m_Textures;
    std::vector<FRDGPass*> m_Passes;
};
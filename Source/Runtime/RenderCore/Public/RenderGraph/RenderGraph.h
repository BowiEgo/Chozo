#pragma once

#include "ImagePool.h"
#include "RHIAPI.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RHITexture.h"
#include "RHITexture2D.h"
#include "RHITextureCubemap.h"
#include "Texture.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRenderGraph, Info);

struct FRDGTexture {
    std::string Name;
    FTextureSpecification Spec;
    TRef<IRHIImage> Image  = nullptr;
    TRef<CTexture> Texture = nullptr;
    bool bExternal = false; // Whether this texture is externally provided (e.g., swapchain image,
    // skybox texture) and shouldn't be pooled

    EImageLayout InitialLayout = EImageLayout::Undefined;
    EImageLayout FinalLayout   = EImageLayout::Undefined;
    EImageLayout CurrentLayout = EImageLayout::Undefined;

    uint32_t FirstPass = 0;
    uint32_t LastPass  = 0;
};

enum class ERenderPassLoadOp { Clear, Load };

class CRenderGraphExecuteContext;

struct FRDGPass {
    std::string Name;
    std::vector<FRDGTexture*> Inputs;
    std::vector<FRDGTexture*> Outputs;
    ERenderPassLoadOp LoadOp;
    std::function<void(CRenderGraphExecuteContext& ctx)> ExecuteFunc;
};

class CRenderGraphExecuteContext {
public:
    CRenderGraphExecuteContext(TRef<IRHICommandList> cmd, FRDGPass* pass)
        : m_Cmd(cmd), m_Pass(pass) {

        for (auto* input : m_Pass->Inputs) {
            m_Resources[input] = input->Texture;
        }
        for (auto* output : m_Pass->Outputs) {
            m_Resources[output] = output->Texture;
        }
    }

    TRef<IRHICommandList> GetCommandBuffer() const { return m_Cmd; }

    TRef<CTexture> GetTexture(FRDGTexture* handle) { return m_Resources[handle]; }

    const std::string& GetPassName() const { return m_Pass->Name; }

private:
    TRef<IRHICommandList> m_Cmd;
    FRDGPass* m_Pass;

    std::unordered_map<FRDGTexture*, TRef<CTexture>> m_Resources;
};

class CRenderGraph {
public:
    CRenderGraph(IRHIContext* ctx) : m_Context(ctx) {}
    ~CRenderGraph();

    // Declare Logical Textures (no physical allocation yet)
    FRDGTexture* CreateRDGTexture(const FTextureSpecification& spec, std::string name) {
        auto RDGtex = new FRDGTexture{ name, spec, nullptr, CreateRef<CTexture>(spec) };
        m_Textures.push_back(RDGtex);
        return RDGtex;
    }

    FRDGTexture* ImportExternalRDGTexture(std::string name, TRef<CTexture> tex,
                                          const EImageLayout initialLayout,
                                          const EImageLayout finalLayout) {
        for (auto* existingTex : m_Textures) {
            if (existingTex->bExternal && existingTex->Spec == tex->GetSpec()) {
                existingTex->FinalLayout = finalLayout;
                return existingTex;
            }
        }

        // CZ_LOG(LogRenderGraph, Trace, "ImportExternalRDGTexture");

        auto RDGtex =
            new FRDGTexture{ name,        tex->GetSpec(), tex->GetOrCreateResource()->GetImage(),
                             tex,         true,           initialLayout,
                             finalLayout, initialLayout };
        m_Textures.push_back(RDGtex);
        return RDGtex;
    }

    // FRDGTexture* ImportExternalRDGTexture(std::string name, const IRHITexture* rhiTex,
    //                                       const EImageLayout initialLayout,
    //                                       const EImageLayout finalLayout) {
    //     for (auto* existingTex : m_Textures) {
    //         if (existingTex->bExternal && existingTex->Spec == rhiTex->GetSpec()) {
    //             existingTex->FinalLayout = finalLayout;
    //             return existingTex;
    //         }
    //     }

    //     // CZ_LOG(LogRenderGraph, Trace, "ImportExternalRDGTexture");

    //     auto tex    = CreateRef<CTexture>(rhiTex->GetSpec(), rhiTex);
    //     auto RDGtex = new FRDGTexture{ name, rhiTex->GetSpec(), rhiTex->GetImage(), tex,
    //                                    true, initialLayout,     finalLayout,        initialLayout
    //                                    };
    //     m_Textures.push_back(RDGtex);
    //     return RDGtex;
    // }

    // Register Passes with their input/output logical textures and execution logic
    void AddPass(std::string name, std::vector<FRDGTexture*> inputs,
                 std::vector<FRDGTexture*> outputs, ERenderPassLoadOp loadOp,
                 std::function<void(CRenderGraphExecuteContext& ctx)> execute) {
        auto pass = new FRDGPass{ name, inputs, outputs, loadOp, execute };
        m_Passes.push_back(pass);

        // Update texture lifetime range
        for (auto* t : inputs)
            t->LastPass = m_Passes.size() - 1;
        for (auto* t : outputs)
            t->LastPass = m_Passes.size() - 1;
    }

    // The most critical step, for physical allocation and barrier insertion
    void Compile() {
        for (auto* tex : m_Textures) {
            if (tex->bExternal) continue;

            // Request a physical Image from the physical resource pool
            // The pool will check if there's a matching
            // and expired Image available for reuse (Aliasing)
            FImageSpecification physSpec = tex->Spec.ToImageSpec();
            tex->Image                   = m_Context->GetDevice()->GetImageFromPool(physSpec);
        }
    }

    // The actual Vulkan calls happen here
    void Execute(TRef<IRHICommandList> cmd) {
        for (uint32_t i = 0; i < m_Passes.size(); ++i) {
            auto* pass = m_Passes[i];
            std::vector<IRHITexture2D*> renderTargets;
            // --- Automatic synchronization logic ---
            for (auto* input : pass->Inputs) {
                // Automatically insert barriers: transition from current state to ShaderReadOnly
                IRHIAPI::TransitionImageLayout(cmd, input->Image,
                                               EImageLayout::ShaderReadOnlyOptimal);
            }
            for (auto* output : pass->Outputs) {
                // Automatically insert barriers: transition to ColorAttachmentOptimal
                IRHIAPI::TransitionImageLayout(cmd, output->Image,
                                               EImageLayout::ColorAttachmentOptimal);

                renderTargets.push_back(
                    static_cast<IRHITexture2D*>(output->Texture->GetOrCreateResource()));
            }

            // Execute user-defined rendering logic for this pass
            m_Context->SetRenderTargets(renderTargets);
            bool shouldClear = (pass->LoadOp == ERenderPassLoadOp::Clear);
            IRHIAPI::BeginRendering(cmd, shouldClear);

            CRenderGraphExecuteContext execCtx(cmd, pass);
            pass->ExecuteFunc(execCtx);
            IRHIAPI::EndRendering(cmd);

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

    std::vector<FRDGTexture*> m_Textures;
    std::vector<FRDGPass*> m_Passes;
};
#pragma once

#include "ImagePool.h"
#include "RHICommandList.h"
#include "RHIContext.h"
#include "RHITexture.h"

struct FRDGTexture {
    std::string Name;
    FTextureSpecification Spec;

    // Pointer to the actual image in the physical pool, null before Compile
    TRef<IRHIImage> Image     = nullptr;
    // Optional: if we want to wrap the image in a higher-level texture abstraction (e.g., for
    // samplers or descriptor sets), we can store it here
    TRef<IRHITexture> Texture = nullptr;

    bool bExternal = false; // Whether this texture is externally provided (e.g., swapchain image,
                            // skybox texture) and shouldn't be pooled

    // Lifetime tracking: earliest and latest Pass index that uses this texture in the graph
    uint32_t FirstPass = 0;
    uint32_t LastPass  = 0;
};

struct FRDGPass {
    std::string Name;
    std::vector<FRDGTexture*> Inputs;
    std::vector<FRDGTexture*> Outputs;
    std::function<void(TRef<IRHICommandList>)> ExecuteFunc;
};

class CRenderGraph {
public:
    CRenderGraph(IRHIContext* context) : m_Context(context) {}
    ~CRenderGraph();

    // Declare Logical Textures (no physical allocation yet)
    FRDGTexture* CreateTexture(const FTextureSpecification& spec, std::string name) {
        auto tex = new FRDGTexture{ name, spec };
        m_Textures.push_back(tex);
        return tex;
    }

    // Register Passes with their input/output logical textures and execution logic
    void AddPass(std::string name, std::vector<FRDGTexture*> inputs,
                 std::vector<FRDGTexture*> outputs,
                 std::function<void(TRef<IRHICommandList>)> execute) {
        auto pass = new FRDGPass{ name, inputs, outputs, execute };
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
            tex->Image = m_Context->GetDevice()->GetImagePool().RequestImage(physSpec);
        }
    }

    // The actual Vulkan calls happen here
    void Execute(TRef<IRHICommandList> cmd) {
        for (auto* pass : m_Passes) {
            // --- Automatic synchronization logic ---
            // for (auto* input : pass->Inputs) {
            //     // Automatically insert barriers: transition from current state to ShaderReadOnly
            //     AddTransitionBarrier(cmd, input->Image,
            //                          vk::ImageLayout::eShaderReadOnlyOptimal);
            // }
            // for (auto* output : pass->Outputs) {
            //     // Automatically insert barriers: transition to ColorAttachmentOptimal
            //     AddTransitionBarrier(cmd, output->Image,
            //                          vk::ImageLayout::eColorAttachmentOptimal);
            // }

            // Execute user-defined rendering logic for this pass
            pass->ExecuteFunc(cmd);
        }
    }

private:
    IRHIContext* m_Context;

    std::vector<FRDGTexture*> m_Textures;
    std::vector<FRDGPass*> m_Passes;
};
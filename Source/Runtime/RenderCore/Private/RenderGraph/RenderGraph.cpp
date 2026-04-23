#include "RenderGraph.h"

CRenderGraph::~CRenderGraph() {
    for (auto& rdgTex : m_InternalResources) {
        if (rdgTex->GetImage()) {
            m_Context->GetDevice()->ReturnImageToPool(rdgTex->GetImage());
        }
    }

    for (auto* tex : m_Textures)
        delete tex;

    for (auto* pass : m_Passes)
        delete pass;

    m_Textures.clear();
    m_Passes.clear();
    m_InternalResources.clear();
}
#include "RenderGraph.h"

CRenderGraph::~CRenderGraph() {
    for (auto& rdgTex : m_InternalResources) {
        if (rdgTex->GetImage()) {
            m_Context->GetDevice()->ReturnImageToPool(rdgTex->GetImage());
        }
    }

    m_InternalResources.clear();
    m_Textures.clear();
    m_Passes.clear();
}
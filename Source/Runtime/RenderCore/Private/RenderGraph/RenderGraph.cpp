#include "RenderGraph.h"

CRenderGraph::~CRenderGraph() {
    for (auto* rdgTex : m_Textures) {
        if (rdgTex->Image && !rdgTex->bExternal) {
            m_Context->GetDevice()->GetImagePool().ReleaseImage(rdgTex->Image);
        }
    }

    m_Textures.clear();
    m_Passes.clear();
}
#include "ImGuiRenderer.h"

DEFINE_LOG_CATEGORY(LogImGuiRenderer);

IImGuiRenderer::IImGuiRenderer(CWindow* window, IRHIContext* rhiContext)
    : m_Window(window), m_RHIContext(rhiContext) {}

IImGuiRenderer::~IImGuiRenderer() {
    CZ_LOG(LogImGuiRenderer, Trace, "ImGuiRenderer destroying...");
}
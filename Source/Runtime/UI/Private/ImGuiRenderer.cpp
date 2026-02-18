#include "ImGuiRenderer.h"

DEFINE_LOG_CATEGORY(LogImGuiRenderer);

IImGuiRenderer::IImGuiRenderer(CWindow* window, CGraphicsContext* context)
    : m_Window(window), m_Context(context) {}

IImGuiRenderer::~IImGuiRenderer() {
    CZ_LOG(LogImGuiRenderer, Trace, "ImGuiRenderer destroying...");
}
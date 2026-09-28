#include "Editor.hpp"

#include <Runtime/App/Application.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

using namespace CZ;

extern "C" {
// Returns a plain pointer so the exported symbol has a C-compatible signature.
StartupHostObj* CreateEditor() { return CZ_NEW(MEMORY_USAGE_RENDER, Editor); }
}

Editor::Editor() {
    m_bOffscreenRendering = true;
    m_StartupLayer        = CZ_NEW(MEMORY_USAGE_RENDER, EditorLayer);
}

Editor::~Editor() {}

void Editor::Draw(CommandList cmdList) { m_StartupLayer->Draw(cmdList); }

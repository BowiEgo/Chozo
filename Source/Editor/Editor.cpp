#include "Editor.hpp"

#include <Runtime/App/Application.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

using namespace CZ;

extern "C" {
StartupHost CreateEditor() { return StartupHost(CZ_NEW(MEMORY_USAGE_RENDER, EditorObj)); }
}

DEFINE_LOG_CATEGORY(LogEditor);

EditorObj::EditorObj() {
    m_bOffscreenRendering = true;
    m_StartupLayer        = CZ_NEW(MEMORY_USAGE_RENDER, EditorLayer);
}

EditorObj::~EditorObj() {}

void EditorObj::Draw(CommandList cmdList) { m_StartupLayer->Draw(cmdList); }

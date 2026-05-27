#pragma once

#include <Runtime/App/Application.hpp>
#include <Runtime/App/StartupHost.hpp>

#include "EditorLayer.hpp"

using namespace CZ;

DECLARE_LOG_CATEGORY_EXTERN(LogEditor, Info);

class Editor : public StartupHostObj {
public:
    Editor();
    ~Editor();

    static VulkanImGuiRenderer* GetImGuiRenderer() {
        return Application::Get().GetStartupHost().As<Editor>()->m_StartupLayer->GetImGuiRenderer();
    }

    static Window GetWindow() { return Application::Get().GetWindow(); }

    Layer* GetStartupLayer() const override { return m_StartupLayer; }

    void Draw(CommandList cmdList) override;

private:
    EditorLayer* m_StartupLayer;
};
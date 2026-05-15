#pragma once
#include <Runtime/App/StartupHost.hpp>

#include "EditorLayer.hpp"

using namespace CZ;

DECLARE_LOG_CATEGORY_EXTERN(LogEditor, Info);

class EditorObj : public StartupHostObj {
public:
    EditorObj();
    ~EditorObj();

    Layer* GetStartupLayer() const override { return m_StartupLayer; }

    void Draw(CommandList cmdList) override;

private:
    EditorLayer* m_StartupLayer;
};
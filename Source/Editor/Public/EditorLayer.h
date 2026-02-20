#pragma once

#include "Chozo.h"
#include "Core.h"
#include "Layer.h"

class EditorLayer : public ILayer {
public:
    EditorLayer();

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(FTimeStep ts) override;
    void OnImGuiRender() override;
    void OnEvent(IEvent& e) override;

private:
};
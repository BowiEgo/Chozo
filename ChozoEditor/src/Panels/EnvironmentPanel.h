#pragma once

#include "Chozo.h"

namespace Chozo {

    class EnvironmentPanel
    {
    public:
        EnvironmentPanel() = default;
        EnvironmentPanel(const Ref<Scene> context);

        void SetContext(const Ref<Scene> scene);

        void OnImGuiRender();
    private:
        Ref<Scene> m_Context;
        bool m_ShowPreview;
    };
}
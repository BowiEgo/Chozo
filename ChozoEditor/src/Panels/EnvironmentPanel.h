#pragma once

#include "Chozo.h"

namespace Chozo {

    class EnvironmentPanel
    {
    public:
        EnvironmentPanel();
        explicit EnvironmentPanel(const Ref<Scene>& context);

        static void SetContext(const Ref<Scene>& scene);

        void OnImGuiRender();
    private:
        static EnvironmentPanel* s_Instance;

        Ref<Scene> m_Context;
        bool m_ShowPreview{};
    };
}
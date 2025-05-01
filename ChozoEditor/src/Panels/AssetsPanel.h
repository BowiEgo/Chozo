#pragma once

namespace Chozo {

    class AssetsPanel
    {
    public:
        AssetsPanel();

        void OnImGuiRender();
    private:
        static AssetsPanel* s_Instance;
    };
}
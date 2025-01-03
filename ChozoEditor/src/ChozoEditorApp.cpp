#define CHOZO_ENTRY_POINT
#include "EditorLayer.h"

namespace Chozo {

    class ChozoEditor : public Application
    {
    public:
        ChozoEditor()
            : Application("Chozo Editor")
        {
            CZ_INFO("Welcome to Chozo Editor!");
            PushLayer(new EditorLayer());
        }

        ~ChozoEditor() override = default;
    };

    Application *CreateApplication()
    {
        return new ChozoEditor();
    };
}

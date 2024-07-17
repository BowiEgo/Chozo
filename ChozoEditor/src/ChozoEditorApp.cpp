#define CHZO_ENTRY_POINT
#include "EditorLayer.h"

namespace Chozo {

    class ChozoEditor : public Application
    {
    public:
        ChozoEditor()
            : Application("Chozo Engine")
        {
            CZ_INFO("Welcome to Chozo Editor!");
            PushLayer(new EditorLayer());
        }

        ~ChozoEditor()
        {
        }
    };

    Application *CreateApplication()
    {
        return new ChozoEditor();
    };
}

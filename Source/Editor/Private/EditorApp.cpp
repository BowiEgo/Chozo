#define CZ_ENTRY_POINT
#include "EditorLayer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogChozoEditor, Info);

class ChozoEditor : public CApplication {
public:
    ChozoEditor() : CApplication("Chozo Editor - Vulkan") {
        CZ_LOG(LogChozoEditor, Info, "Welcome to Chozo Editor!");
        PushLayer(new EditorLayer());
    }

    ~ChozoEditor() override = default;
};

CApplication* CreateApplication() { return new ChozoEditor(); };

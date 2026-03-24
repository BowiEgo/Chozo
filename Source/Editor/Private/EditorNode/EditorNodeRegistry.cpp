#include "EditorNodeRegistry.h"

FEditorNodeRegistry& FEditorNodeRegistry::Get() {
    static FEditorNodeRegistry Instance;
    return Instance;
}
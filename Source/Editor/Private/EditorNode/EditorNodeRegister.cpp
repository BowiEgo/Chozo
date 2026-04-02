#include "EditorNodeRegister.h"

FEditorNodeRegister& FEditorNodeRegister::Get() {
    static FEditorNodeRegister Instance;
    return Instance;
}

void FEditorNodeRegister::Init() { Get(); }
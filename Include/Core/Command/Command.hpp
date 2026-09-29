#pragma once
#include <string_view>

namespace CZ {

/// One undoable edit. A command owns whatever state it needs to reverse itself, so nothing else in
/// the editor has to know how an edit is undone.
///
/// Apply() performs the edit the first time, Undo() reverses it and Redo() (defaulting to Apply)
/// repeats it. Label() is the short description the toolbar and menus show, for example
/// "Set Sphere.Radius".
///
/// Header-only on purpose: the interface and the stack below are small, dependency-free pieces, so
/// they need no new Core module (and therefore no add_subdirectory wiring).
class Command {
public:
    virtual ~Command() = default;

    virtual void Apply() = 0;
    virtual void Undo()  = 0;
    virtual void Redo() { Apply(); }

    virtual std::string_view Label() const = 0;
};

} // namespace CZ

#pragma once

#include <Core/Log/LogMacros.hpp>
#include <cstddef>
#include <deque>
#include <memory>
#include <string_view>
#include <utility>

#include <Core/Command/Command.hpp>

namespace CZ {

/// Undo/redo history. Executing a command clears the redo branch (that edit is no longer reachable)
/// and the history is bounded, so a long session cannot grow without limit.
///
/// The stack never inspects a command's contents: parameter edits, node creation and deletion all
/// look identical from here, which is what keeps it independent of the editor.
class CommandStack {
public:
    static constexpr size_t kMaxDepth = 256;

    /// Executes the command and pushes it onto the undo history.
    void Execute(std::unique_ptr<Command> command) {
        if (!command) {
            return;
        }

        command->Apply();
        m_Redo.clear();
        m_Undo.push_back(std::move(command));

        while (m_Undo.size() > kMaxDepth) {
            m_Undo.pop_front();
        }
    }

    bool CanUndo() const { return !m_Undo.empty(); }
    bool CanRedo() const { return !m_Redo.empty(); }

    bool Undo() {
        if (m_Undo.empty()) {
            return false;
        }

        std::unique_ptr<Command> command = std::move(m_Undo.back());
        m_Undo.pop_back();
        command->Undo();
        m_Redo.push_back(std::move(command));
        return true;
    }

    bool Redo() {
        if (m_Redo.empty()) {
            return false;
        }

        std::unique_ptr<Command> command = std::move(m_Redo.back());
        m_Redo.pop_back();
        command->Redo();
        m_Undo.push_back(std::move(command));
        return true;
    }

    /// Label of the next undo/redo entry, empty when unavailable (menus and tooltips use it).
    std::string_view UndoLabel() const {
        return m_Undo.empty() ? std::string_view{} : m_Undo.back()->Label();
    }
    std::string_view RedoLabel() const {
        return m_Redo.empty() ? std::string_view{} : m_Redo.back()->Label();
    }

    void Clear() {
        m_Undo.clear();
        m_Redo.clear();
    }

    size_t UndoDepth() const { return m_Undo.size(); }
    size_t RedoDepth() const { return m_Redo.size(); }

private:
    std::deque<std::unique_ptr<Command>> m_Undo;
    std::deque<std::unique_ptr<Command>> m_Redo;
};

} // namespace CZ

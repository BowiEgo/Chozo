#pragma once

#include "Panel.hpp"

#include <Core/Log/LogScopedSink.hpp>

using namespace CZ;

struct LogLine {
    char* Text;
    LogVerbosity Level;
};

class ConsolePanel : public Panel {
public:
    ConsolePanel();
    ~ConsolePanel();

    virtual void Draw(const char* title) override;

    void ClearLog();
    void AddLog(const char* message, LogVerbosity level);
    void ExecCommand(const char* command_line);
    int TextEditCallback(ImGuiInputTextCallbackData* data);

    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

private:
    ImVec4 GetColorForLevel(LogVerbosity level);

private:
    char m_InputBuf[256];
    ImVector<LogLine> m_Items;
    ImVector<const char*> m_Commands;
    ImVector<char*> m_History;
    int m_HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter m_Filter;
    bool m_AutoScroll;
    bool m_ScrollToBottom;

    std::mutex m_LogMutex;
    Scope<LogScopedSink> m_LogSink;
};
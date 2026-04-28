#pragma once

#include "Panel.h"

#include "CoreMinimal.h"
#include "LogScopedSink.h"
#include "Scope.h"

DECLARE_LOG_CATEGORY_EXTERN(LogConsolePanel, Info);

struct LogLine {
    char* Text;
    ELogVerbosity Level;
};

class ConsolePanel : public Panel {
public:
    ConsolePanel();
    ~ConsolePanel();

    virtual void Draw(const char* title) override;

    void ClearLog();
    void AddLog(const char* message, ELogVerbosity level);
    void ExecCommand(const char* command_line);
    int TextEditCallback(ImGuiInputTextCallbackData* data);

    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

private:
    ImVec4 GetColorForLevel(ELogVerbosity level);

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
    TScope<FLogScopedSink> m_LogSink;
};
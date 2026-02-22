#pragma once

#include "Panel.h"

class ConsolePanel : public Panel {
public:
    ConsolePanel();
    ~ConsolePanel();

    virtual void Draw(const char* title, bool* p_open) override;

    void ClearLog();
    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void ExecCommand(const char* command_line);
    int TextEditCallback(ImGuiInputTextCallbackData* data);

    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

private:
    char m_InputBuf[256];
    ImVector<char*> m_Items;
    ImVector<const char*> m_Commands;
    ImVector<char*> m_History;
    int m_HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter m_Filter;
    bool m_AutoScroll;
    bool m_ScrollToBottom;
};
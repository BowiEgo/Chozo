#include "Window.h"

namespace Chozo
{
    FWindow::FWindow(const FWindowDefinition& windowDef)
    {
        Init(windowDef);
    }

    void FWindow::Init(const FWindowDefinition& windowDef)
    {
        m_Data.Title = windowDef.Title;
        m_Data.Width = windowDef.Width;
        m_Data.Height = windowDef.Height;
    }
}
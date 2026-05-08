#include "Application.h"

#include <iostream>

namespace CZ {

DEFINE_LOG_CATEGORY(LogApplication);

Application::Application() {}

Application::~Application() {}

bool Application::Startup(const ApplicationInfo& info, std::string& err) {
    m_bRunning = true;
    return true;
}

void Application::Cleanup() {}

void Application::Run() {
    while (m_bRunning) {
        CZ_LOG(LogApplication, Trace, "Running...");
    }
}

} // namespace CZ
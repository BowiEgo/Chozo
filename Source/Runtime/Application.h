#pragma once

#include "Core/Log/LogMacros.h"

#include <string>

namespace CZ {

DECLARE_LOG_CATEGORY_EXTERN(LogApplication, Info);

struct ApplicationInfo {
    /* data */
};

class Application {
public:
    Application();
    Application(const Application&) = delete;
    ~Application();

    Application& operator=(const Application&) = delete;

    bool Startup(const ApplicationInfo& info, std::string& err);
    void Cleanup();

    void Run();

private:
    bool m_bRunning;
    // RuntimeContext mRuntimeCtx = {};
};

} // namespace CZ

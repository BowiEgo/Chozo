#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "Chozo/Events/ApplicationEvent.h"
#include "Chozo/Events/MouseEvent.h"
#include "Chozo/Events/KeyEvent.h"
#include "FmtFormatter.h"

namespace Chozo
{
    class CHOZO_API Log
    {
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger> &GetClientLogger() { return s_ClientLogger; }
    };
}

// Core log macros
#define CZ_CORE_TRACE(...) ::Chozo::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CZ_CORE_INFO(...)  ::Chozo::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CZ_CORE_WARN(...)  ::Chozo::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CZ_CORE_ERROR(...) ::Chozo::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CZ_CORE_FATAL(...) ::Chozo::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define CZ_TRACE(...) ::Chozo::Log::GetClientLogger()->trace(__VA_ARGS__)
#define CZ_INFO(...)  ::Chozo::Log::GetClientLogger()->info(__VA_ARGS__)
#define CZ_WARN(...)  ::Chozo::Log::GetClientLogger()->warn(__VA_ARGS__)
#define CZ_ERROR(...) ::Chozo::Log::GetClientLogger()->error(__VA_ARGS__)
#define CZ_FATAL(...) ::Chozo::Log::GetClientLogger()->fatal(__VA_ARGS__)

#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "Parallax/Events/ApplicationEvent.h"
#include "Parallax/Events/MouseEvent.h"
#include "Parallax/Events/KeyEvent.h"
#include "Parallax/FmtFormatter.h"

namespace Parallax
{
    class PARALLAX_API Log
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
#define PRX_CORE_TRACE(...) ::Parallax::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PRX_CORE_INFO(...)  ::Parallax::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PRX_CORE_WARN(...)  ::Parallax::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PRX_CORE_ERROR(...) ::Parallax::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PRX_CORE_FATAL(...) ::Parallax::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define PRX_TRACE(...) ::Parallax::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PRX_INFO(...)  ::Parallax::Log::GetClientLogger()->info(__VA_ARGS__)
#define PRX_WARN(...)  ::Parallax::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PRX_ERROR(...) ::Parallax::Log::GetClientLogger()->error(__VA_ARGS__)
#define PRX_FATAL(...) ::Parallax::Log::GetClientLogger()->fatal(__VA_ARGS__)

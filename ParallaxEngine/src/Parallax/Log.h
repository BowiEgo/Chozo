#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"

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
#define PX_CORE_TRACE(...) ::Parallax::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PX_CORE_INFO(...) ::Parallax::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PX_CORE_WARN(...) ::Parallax::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PX_CORE_ERROR(...) ::Parallax::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PX_CORE_FATAL(...) ::Parallax::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define PX_TRACE(...) ::Parallax::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PX_INFO(...) ::Parallax::Log::GetClientLogger()->info(__VA_ARGS__)
#define PX_WARN(...) ::Parallax::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PX_ERROR(...) ::Parallax::Log::GetClientLogger()->error(__VA_ARGS__)
#define PX_FATAL(...) ::Parallax::Log::GetClientLogger()->fatal(__VA_ARGS__)

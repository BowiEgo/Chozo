#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "FmtFormatter.h" // NOLINT

namespace Chozo
{
    class Log
    {
    public:
        enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};
    public:
        static void Init();

        template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args);

        static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger> &GetClientLogger() { return s_ClientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;
    };
}

// Core logging
#define CZ_CORE_TRACE_TAG(tag, ...) ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Core, ::Chozo::Log::Level::Trace, tag, __VA_ARGS__)
#define CZ_CORE_INFO_TAG(tag, ...)  ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Core, ::Chozo::Log::Level::Info, tag, __VA_ARGS__)
#define CZ_CORE_WARN_TAG(tag, ...)  ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Core, ::Chozo::Log::Level::Warn, tag, __VA_ARGS__)
#define CZ_CORE_ERROR_TAG(tag, ...) ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Core, ::Chozo::Log::Level::Error, tag, __VA_ARGS__)
#define CZ_CORE_FATAL_TAG(tag, ...) ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Core, ::Chozo::Log::Level::Fatal, tag, __VA_ARGS__)

// Client logging
#define CZ_TRACE_TAG(tag, ...) ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Client, ::Chozo::Log::Level::Trace, tag, __VA_ARGS__)
#define CZ_INFO_TAG(tag, ...)  ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Client, ::Chozo::Log::Level::Info, tag, __VA_ARGS__)
#define CZ_WARN_TAG(tag, ...)  ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Client, ::Chozo::Log::Level::Warn, tag, __VA_ARGS__)
#define CZ_ERROR_TAG(tag, ...) ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Client, ::Chozo::Log::Level::Error, tag, __VA_ARGS__)
#define CZ_FATAL_TAG(tag, ...) ::Chozo::Log::PrintMessage(::Chozo::Log::Type::Client, ::Chozo::Log::Level::Fatal, tag, __VA_ARGS__)

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

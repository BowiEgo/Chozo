#pragma once

#include "Core/Log/LogEnum.h"

#include <spdlog/spdlog.h>

namespace CZ::LogUtils {

static spdlog::level::level_enum ToSpdlogLevel(LogVerbosity verbosity) {
    switch (verbosity) {
        case LogVerbosity::Fatal: return spdlog::level::critical;
        case LogVerbosity::Error: return spdlog::level::err;
        case LogVerbosity::Warning: return spdlog::level::warn;
        case LogVerbosity::Info: return spdlog::level::info;
        case LogVerbosity::Debug: return spdlog::level::debug;
        case LogVerbosity::Trace: return spdlog::level::trace;
        default: return spdlog::level::info;
    }
}

static LogVerbosity FromSpdlogLevel(spdlog::level::level_enum level) {
    switch (level) {
        case spdlog::level::critical: return LogVerbosity::Fatal;
        case spdlog::level::err: return LogVerbosity::Error;
        case spdlog::level::warn: return LogVerbosity::Warning;
        case spdlog::level::info: return LogVerbosity::Info;
        case spdlog::level::debug: return LogVerbosity::Debug;
        case spdlog::level::trace: return LogVerbosity::Trace;
        default: return LogVerbosity::Info;
    }
}
} // namespace CZ::LogUtils
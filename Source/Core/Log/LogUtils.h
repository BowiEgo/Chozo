#pragma once

#include "Core/Log/LogEnum.h"

#include <spdlog/spdlog.h>

namespace CZ::LogUtils {

static spdlog::level::level_enum ToSpdlogLevel(ELogVerbosity verbosity) {
    switch (verbosity) {
        case ELogVerbosity::Fatal: return spdlog::level::critical;
        case ELogVerbosity::Error: return spdlog::level::err;
        case ELogVerbosity::Warning: return spdlog::level::warn;
        case ELogVerbosity::Info: return spdlog::level::info;
        case ELogVerbosity::Debug: return spdlog::level::debug;
        case ELogVerbosity::Trace: return spdlog::level::trace;
        default: return spdlog::level::info;
    }
}

static ELogVerbosity FromSpdlogLevel(spdlog::level::level_enum level) {
    switch (level) {
        case spdlog::level::critical: return ELogVerbosity::Fatal;
        case spdlog::level::err: return ELogVerbosity::Error;
        case spdlog::level::warn: return ELogVerbosity::Warning;
        case spdlog::level::info: return ELogVerbosity::Info;
        case spdlog::level::debug: return ELogVerbosity::Debug;
        case spdlog::level::trace: return ELogVerbosity::Trace;
        default: return ELogVerbosity::Info;
    }
}
} // namespace CZ::LogUtils
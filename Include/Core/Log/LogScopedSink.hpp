#pragma once

#include "LogEnum.hpp"

namespace CZ {

class LogScopedSink {
public:
    LogScopedSink(LogCallback callback, const std::string& pattern);
    ~LogScopedSink();

    LogScopedSink(const LogScopedSink&)            = delete;
    LogScopedSink& operator=(const LogScopedSink&) = delete;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

} // namespace CZ

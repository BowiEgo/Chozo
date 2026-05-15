#pragma once

#include "LogEnum.hpp"
#include "Logger.hpp"

namespace CZ {

class FLogScopedSink {
public:
    FLogScopedSink(LogCallback callback, const std::string& pattern);
    ~FLogScopedSink();

    FLogScopedSink(const FLogScopedSink&)            = delete;
    FLogScopedSink& operator=(const FLogScopedSink&) = delete;

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

} // namespace CZ

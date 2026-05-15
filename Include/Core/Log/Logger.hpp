#pragma once

#include "LogEnum.hpp"
#include <Core/Header/Macros.h>

#include <functional>
#include <mutex>
#include <string>

namespace CZ {

class Logger {
public:
    static Logger& Get();

    using SinkHandle = std::shared_ptr<void>;
    static SinkHandle
        AddCallbackSink(std::function<void(const std::string&, LogVerbosity)> callback,
                        const std::string& pattern = "%v");
    static void RemoveSink(SinkHandle handle);

    /**
     * The core log function that dispatches messages to sinks
     */
    void Log(const std::string& category, LogVerbosity verbosity, const std::string& message);

private:
    Logger();
    ~Logger();

    // Forward declaration of internal spdlog implementation (Pimpl pattern)
    struct FImpl;
    std::unique_ptr<FImpl> Impl;
};

} // namespace CZ

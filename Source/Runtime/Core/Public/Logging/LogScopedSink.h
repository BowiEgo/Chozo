#include "Logger.h"

class FLogScopedSink {
public:
    FLogScopedSink(LogCallbackSink_mt::LogCallback callback, const std::string& pattern) {
        m_Sink = FLogger::AddCallbackSink(std::move(callback), pattern);
    }

    ~FLogScopedSink() { FLogger::RemoveSink(m_Sink); }

private:
    spdlog::sink_ptr m_Sink;
};
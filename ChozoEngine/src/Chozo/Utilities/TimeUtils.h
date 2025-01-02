#pragma once

namespace Chozo::Utils::Time {

    inline uint64_t CreateTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        return timestamp;
    }

    inline std::string FormatTimestamp(const uint64_t timestamp) {
        const std::time_t time = timestamp;
        const std::tm* tm = std::localtime(&time);

        if (!tm) return "";

        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M");
        return oss.str();
    }
}

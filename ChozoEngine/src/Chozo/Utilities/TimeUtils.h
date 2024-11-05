#pragma once

#include <chrono>

namespace Chozo::Utils {

    namespace Time {

		inline uint64_t CreateTimestamp()
		{
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
			return timestamp;
		}

        inline std::string FormatTimestamp(uint64_t timestamp) {
            std::time_t time = static_cast<std::time_t>(timestamp);
            std::tm* tm = std::localtime(&time);

            if (!tm) return "";

            std::ostringstream oss;
            oss << std::put_time(tm, "%Y-%m-%d/%H::%M");
            return oss.str();
        }
    }
}
#pragma once

#include <codecvt>

#if defined(CZ_PLATFORM_WIN)
#include <windows.h>
#else
#endif

namespace Chozo::Utils {

namespace String {

inline bool isNumeric(const std::string &str) {
    std::regex pattern(R"(^\d+$)");
    return std::regex_match(str, pattern);
}

inline std::string &ToLower(std::string &string) {
    std::transform(string.begin(), string.end(), string.begin(),
        [](const unsigned char c) { return std::tolower(c); });
    return string;
}

inline std::string ToLowerCopy(const std::string_view string) {
    std::string result(string);
    ToLower(result);
    return result;
}

inline uint64_t ToUint64(std::string string) {
    uint64_t result = isNumeric(string) ? std::stoull(string) : 0;
    return result;
}
} // namespace String

namespace Numeric {
inline std::string FormatWithCommas(uint64_t value) {
    std::stringstream ss;
    ss.imbue(std::locale("en_US.UTF-8")); // Locale adds comma grouping
    ss << std::fixed << value;
    return ss.str();
}
} // namespace Numeric

namespace WChar {

} // namespace WChar
} // namespace Chozo::Utils
#include "StringUtils.h"

#include <codecvt>
#include <regex>

namespace ChozoUtils {

namespace String {

bool isNumeric(const std::string& str) {
    std::regex pattern(R"(^\d+$)");
    return std::regex_match(str, pattern);
}

std::string& ToLower(std::string& string) {
    std::transform(string.begin(), string.end(), string.begin(),
                   [](const unsigned char c) { return std::tolower(c); });
    return string;
}

std::string ToLowerCopy(const std::string_view string) {
    std::string result(string);
    ToLower(result);
    return result;
}

uint64_t ToUint64(std::string string) {
    uint64_t result = isNumeric(string) ? std::stoull(string) : 0;
    return result;
}

std::string RemovePrefix(const std::string& str, const std::string& prefix) {
    if (str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0)
        return str.substr(prefix.size());
    return str;
}

std::string TrimQuotes(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}
} // namespace String

namespace Numeric {

std::string FormatWithCommas(uint64_t value) {
    std::stringstream ss;
    ss.imbue(std::locale("en_US.UTF-8")); // Locale adds comma grouping
    ss << std::fixed << value;
    return ss.str();
}

} // namespace Numeric

namespace WChar {} // namespace WChar

} // namespace ChozoUtils
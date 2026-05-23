#pragma once

#include <regex>

namespace CZ::StringUtils {

bool isNumeric(const std::string& str);

std::string& ToLower(std::string& string);

std::string ToLowerCopy(const std::string_view string);

uint64_t ToUint64(std::string string);

std::string RemovePrefix(const std::string& str, const std::string& prefix);

std::string TrimQuotes(const std::string& str);

} // namespace CZ::StringUtils
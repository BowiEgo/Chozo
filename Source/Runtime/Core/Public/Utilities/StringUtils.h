#pragma once

#include "CoreMinimal.h"
#include "Platform.h"

#include <codecvt>
#include <regex>

namespace ChozoUtils {

namespace String {

CORE_API bool isNumeric(const std::string& str);

CORE_API std::string& ToLower(std::string& string);

CORE_API std::string ToLowerCopy(const std::string_view string);

CORE_API uint64_t ToUint64(std::string string);

CORE_API std::string RemovePrefix(const std::string& str, const std::string& prefix);

} // namespace String

namespace Numeric {

CORE_API std::string FormatWithCommas(uint64_t value);

} // namespace Numeric

namespace WChar {} // namespace WChar

} // namespace ChozoUtils
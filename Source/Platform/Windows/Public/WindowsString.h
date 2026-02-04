#pragma once

#include <cstdint>
#include <string>

namespace ChozoPlatform {

namespace WChar {

std::string WStringToString(const std::wstring &wstr);

uint64_t WCharToUint64(const wchar_t *wchar_str);

} // namespace WChar
} // namespace ChozoPlatform
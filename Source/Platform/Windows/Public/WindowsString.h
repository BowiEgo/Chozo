#pragma once

namespace ChozoUtils {

namespace String {

std::string WStringToString(const std::wstring& wstr);

std::wstring StringToWString(const std::string& str);

uint64_t WCharToUint64(const wchar_t* wchar_str);

} // namespace String
} // namespace ChozoUtils
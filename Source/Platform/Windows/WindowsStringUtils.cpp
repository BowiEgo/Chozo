#include <codecvt>
#include <string>
#include <windows.h>

namespace ChozoUtils {

namespace WChar {

inline std::string WStringToString(const std::wstring &wstr) {
    if (wstr.empty())
        return {};
    int size_needed = WideCharToMultiByte(
        CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0],
                        size_needed, nullptr, nullptr);
    return strTo;
}

inline uint64_t WCharToUint64(const wchar_t *wchar_str) {
    if (!wchar_str)
        return 0;

    std::wstring wstr(wchar_str);
    std::string str = WStringToString(wstr);
    return std::stoull(str);
}
} // namespace WChar
} // namespace ChozoUtils
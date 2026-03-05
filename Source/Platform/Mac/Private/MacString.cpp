#include "MacString.h"

#include <codecvt>
#include <string>

namespace ChozoUtils {

namespace String {

uint64_t WCharToUint64(const wchar_t* wchar_str) {
    if (!wchar_str) return 0;

    return std::wcstoull(wchar_str, nullptr, 10);
}

} // namespace String
} // namespace ChozoUtils
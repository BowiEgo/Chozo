#include "WindowsSystem.h"

#include <windows.h>

#include <lmcons.h>

namespace ChozoUtils::System {

std::wstring GetCurrentUserName() {
    wchar_t username[UNLEN + 1] = { 0 };
    DWORD username_len = UNLEN + 1;
    GetUserNameW(username, &username_len);

    return std::wstring(username);
}

} // namespace ChozoUtils::System
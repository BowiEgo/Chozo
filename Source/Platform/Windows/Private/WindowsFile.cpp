#include "WindowsFile.h"

#include <windows.h> // MUST be included BEFORE shellapi.h

#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() {
    wchar_t buffer[MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).lexically_normal();
}

FRawIcon GetIcon(const std::filesystem::path& path) {
    FRawIcon result;
    std::string pathU8 = path.string();
    std::wstring pathW = path.wstring();
    std::replace(pathW.begin(), pathW.end(), L'/', L'\\');

    result.PathU8 = pathU8;

    std::error_code ec;

    SHFILEINFOW fileInfo = { 0 };
    UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
    DWORD attrs = FILE_ATTRIBUTE_NORMAL;

    if (!std::filesystem::exists(path, ec)) {
        flags |= SHGFI_USEFILEATTRIBUTES;
        attrs = FILE_ATTRIBUTE_DIRECTORY;
    }

    if (SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags)) {
        std::string narrowPath = std::filesystem::path(pathW).string();

        ICONINFO iconInfo = { 0 };
        if (GetIconInfo(fileInfo.hIcon, &iconInfo)) {
            if (iconInfo.hbmColor) {
                DIBSECTION ds;
                GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
                int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

                if (byteSize > 0) {
                    uint8_t* data = (uint8_t*)malloc(byteSize);
                    GetBitmapBits(iconInfo.hbmColor, byteSize, data);
                    result.Data = data;
                    result.Size = byteSize;
                    result.Width = ds.dsBm.bmWidth;
                    result.Height = ds.dsBm.bmHeight;
                    result.Indice = fileInfo.iIcon;
                }
                DeleteObject(iconInfo.hbmColor);
            }
            if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
        }
        if (fileInfo.hIcon) DestroyIcon(fileInfo.hIcon);
    }

    return result;
}

bool IsHiddenOrSystem(const std::filesystem::path& p) {
    std::string filename = p.filename().string();

    DWORD attributes = GetFileAttributesW(p.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        if (attributes & FILE_ATTRIBUTE_HIDDEN || attributes & FILE_ATTRIBUTE_SYSTEM ||
            attributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) {
            return true;
        }
    }

    const std::vector<std::string> winSystemFolders = {
        "$Recycle.Bin", "$RECYCLE.BIN", "System Volume Information",
        "Config.Msi",   "$WinREAgent",  "MSOCache",
        "Recovery",     "Windows.old"
    };

    for (const auto& folder : winSystemFolders) {
        if (filename == folder) {
            return true;
        }
    }

    return false;
}

} // namespace ChozoUtils::File
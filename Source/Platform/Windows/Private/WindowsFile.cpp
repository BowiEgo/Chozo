#include "WindowsFile.h"

#include <windows.h> // MUST be included BEFORE shellapi.h

#include <shellapi.h>
#include <shlobj.h>

#pragma comment(lib, "Shell32.lib")

static SHFILEINFOW GetFileInfo(const std::filesystem::path& path) {
    std::wstring pathW = path.wstring();
    std::replace(pathW.begin(), pathW.end(), L'/', L'\\');

    SHFILEINFOW fileInfo = { 0 };
    UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
    DWORD attrs = FILE_ATTRIBUTE_NORMAL;

    if (!std::filesystem::exists(path)) {
        flags |= SHGFI_USEFILEATTRIBUTES;
        attrs = FILE_ATTRIBUTE_DIRECTORY;
    }

    SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags);

    return fileInfo;
}

static HBITMAP GetFileThumbnailBITMAP(const std::wstring& filePath, int size) {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) return nullptr;

    IShellItem* pItem = nullptr;
    HBITMAP hBitmap = nullptr;

    std::wstring normalizedPath = filePath;
    std::replace(normalizedPath.begin(), normalizedPath.end(), L'/', L'\\');

    DWORD attr = GetFileAttributesW(normalizedPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        DWORD err = GetLastError();
    }

    hr = SHCreateItemFromParsingName(normalizedPath.c_str(), nullptr, IID_PPV_ARGS(&pItem));
    if (SUCCEEDED(hr)) {
        IShellItemImageFactory* pFactory = nullptr;
        hr = pItem->QueryInterface(IID_PPV_ARGS(&pFactory));
        if (SUCCEEDED(hr)) {
            SIZE desiredSize = { size, size };
            HBITMAP hThumb = nullptr;

            hr = pFactory->GetImage(desiredSize, SIIGBF_BIGGERSIZEOK | SIIGBF_RESIZETOFIT, &hThumb);
            if (SUCCEEDED(hr) && hThumb) {
                hBitmap = hThumb;
            }
            pFactory->Release();
        }
        pItem->Release();
    } else {
        LPWSTR msgBuf = nullptr;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuf, 0, nullptr);
        LocalFree(msgBuf);
    }

    CoUninitialize();
    return hBitmap;
}

static void GetRawFileImageFromHBitmap(FRawFileImage& rawImage, HBITMAP hBitmap) {
    if (!hBitmap) return;

    DIBSECTION ds = {};
    BITMAP bm = {};

    int width, height, bpp, byteSize;
    if (GetObject(hBitmap, sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION)) {
        width = ds.dsBm.bmWidth;
        height = ds.dsBm.bmHeight;
        bpp = ds.dsBm.bmBitsPixel;
    }

    if (GetObject(hBitmap, sizeof(BITMAP), &bm) == sizeof(BITMAP)) {
        width = bm.bmWidth;
        height = bm.bmHeight;
        bpp = bm.bmBitsPixel;
    }

    byteSize = width * height * (bpp / 8);
    if (byteSize > 0) {
        uint8_t* data = (uint8_t*)malloc(byteSize);
        if (data) {
            if (GetBitmapBits(hBitmap, byteSize, data) == byteSize) {
                rawImage.Data = data;
                rawImage.Size = byteSize;
                rawImage.Width = width;
                rawImage.Height = height;
            } else {
                free(data);
            }
        }
    }
}

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() {
    wchar_t buffer[MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).lexically_normal();
}

int GetFileIconIndex(const std::filesystem::path& path) {
    SHFILEINFOW fileInfo = GetFileInfo(path);

    return fileInfo.iIcon;
}

FRawFileImage GetFileIcon(const std::filesystem::path& path) {
    FRawFileImage result = {};
    result.PathU8 = path.string();

    SHFILEINFOW fileInfo = GetFileInfo(path);

    ICONINFO iconInfo = { 0 };
    if (GetIconInfo(fileInfo.hIcon, &iconInfo)) {
        if (iconInfo.hbmColor) {
            GetRawFileImageFromHBitmap(result, iconInfo.hbmColor);
            result.Index = fileInfo.iIcon;

            DeleteObject(iconInfo.hbmColor);
            if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
        }
    }
    DestroyIcon(fileInfo.hIcon);

    return result;
}

FRawFileImage GetFileThumbnail(const std::filesystem::path& path, int size) {
    FRawFileImage result = {};
    result.PathU8 = path.string();

    HBITMAP hBitmap = GetFileThumbnailBITMAP(path.wstring(), size);
    if (hBitmap) {
        GetRawFileImageFromHBitmap(result, hBitmap);
        DeleteObject(hBitmap);
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
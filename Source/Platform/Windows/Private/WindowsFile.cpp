#include "WindowsFile.h"

#include <windows.h> // MUST be included BEFORE shellapi.h

#include <shellapi.h>
#include <shlobj.h>

#pragma comment(lib, "Shell32.lib")

static bool ShouldUseUniqueIndex(const std::filesystem::path& path) {
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) return true;

    // std::string ext = path.extension().string();
    // if (ext == ".jpg" || ext == ".png") return true;

    return false;
}

static SHFILEINFOW GetFileInfo(const std::filesystem::path& path) {
    std::wstring pathW = path.wstring();
    std::replace(pathW.begin(), pathW.end(), L'/', L'\\');

    SHFILEINFOW fileInfo = { 0 };
    UINT flags           = SHGFI_ICON | SHGFI_LARGEICON;
    DWORD attrs          = FILE_ATTRIBUTE_NORMAL;

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
    HBITMAP hBitmap   = nullptr;

    std::wstring normalizedPath = filePath;
    std::replace(normalizedPath.begin(), normalizedPath.end(), L'/', L'\\');

    DWORD attr = GetFileAttributesW(normalizedPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        DWORD err = GetLastError();
    }

    hr = SHCreateItemFromParsingName(normalizedPath.c_str(), nullptr, IID_PPV_ARGS(&pItem));
    if (SUCCEEDED(hr)) {
        IShellItemImageFactory* pFactory = nullptr;
        hr                               = pItem->QueryInterface(IID_PPV_ARGS(&pFactory));
        if (SUCCEEDED(hr)) {
            SIZE desiredSize = { size, size };
            HBITMAP hThumb   = nullptr;

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

    BITMAP bm;
    if (GetObject(hBitmap, sizeof(BITMAP), &bm)) {
        // [Note] Forces 32-bit (BGRA) regardless of source HBITMAP format
        int width    = bm.bmWidth;
        int height   = bm.bmHeight;
        int byteSize = width * height * 4;

        uint8_t* data = (uint8_t*)malloc(byteSize);
        if (!data) return;

        BITMAPINFO bmi              = {};
        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = width;
        bmi.bmiHeader.biHeight      = -height; // [Note] Negative height for top-down DIB
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        HDC hdc = GetDC(NULL);
        if (GetDIBits(hdc, hBitmap, 0, height, data, &bmi, DIB_RGB_COLORS)) {
            rawImage.Data   = data;
            rawImage.Width  = width;
            rawImage.Height = height;
            rawImage.Size   = byteSize;
        } else {
            free(data);
        }
        ReleaseDC(NULL, hdc);
    }
}

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() {
    wchar_t buffer[MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).lexically_normal();
}

int GetFileIconIndex(const std::filesystem::path& path) {
    if (path.empty()) return -1;

    // [Note] If it's a "Live" item, return the path hash to ensure uniqueness in Vulkan
    if (ShouldUseUniqueIndex(path)) {
        return static_cast<int>(std::hash<std::string>{}(path.string()));
    }

    // [Note] For regular files, return the shared system shell index
    SHFILEINFOW fileInfo = GetFileInfo(path);
    return fileInfo.iIcon;
}

// FRawFileImage GetFileIcon(const std::filesystem::path& path) {
//     FRawFileImage result = {};
//     result.PathU8 = path.string();

//     SHFILEINFOW fileInfo = GetFileInfo(path);

//     ICONINFO iconInfo = { 0 };
//     if (GetIconInfo(fileInfo.hIcon, &iconInfo)) {
//         if (iconInfo.hbmColor) {
//             GetRawFileImageFromHBitmap(result, iconInfo.hbmColor);
//             result.Index = fileInfo.iIcon;

//             DeleteObject(iconInfo.hbmColor);
//             if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
//         }
//     }
//     DestroyIcon(fileInfo.hIcon);

//     return result;
// }

FRawFileImage GetFileIcon(const std::filesystem::path& path) {
    FRawFileImage result = {};
    result.PathU8        = path.string();
    result.Index         = GetFileIconIndex(path);

    std::wstring pathW = path.wstring();
    std::replace(pathW.begin(), pathW.end(), L'/', L'\\');

    // [Note] COM Initialization must be balanced
    HRESULT hrCo       = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    HBITMAP hThumbnail = NULL;

    if (ShouldUseUniqueIndex(path)) {
        IShellItem* pItem = NULL;
        if (SUCCEEDED(SHCreateItemFromParsingName(pathW.c_str(), NULL, IID_PPV_ARGS(&pItem)))) {
            IShellItemImageFactory* pFactory = NULL;
            if (SUCCEEDED(pItem->QueryInterface(IID_PPV_ARGS(&pFactory)))) {
                pFactory->GetImage({ 64, 64 }, SIIGBF_BIGGERSIZEOK, &hThumbnail);
                pFactory->Release();
            }
            pItem->Release();
        }
    }

    if (hThumbnail) {
        GetRawFileImageFromHBitmap(result, hThumbnail);
        DeleteObject(hThumbnail); // [Critical] Delete immediately after conversion
    } else {
        // Fallback to static shell icon
        SHFILEINFOW sfi = { 0 };
        if (SHGetFileInfoW(pathW.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON)) {
            ICONINFO iconInfo = { 0 };
            if (GetIconInfo(sfi.hIcon, &iconInfo)) {
                // GetIconInfo creates NEW bitmaps, must delete both!
                if (iconInfo.hbmColor) {
                    GetRawFileImageFromHBitmap(result, iconInfo.hbmColor);
                    DeleteObject(iconInfo.hbmColor);
                }
                if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
            }
            DestroyIcon(sfi.hIcon); // [Critical] Destroy the icon handle
        }
    }

    if (SUCCEEDED(hrCo)) CoUninitialize();
    return result;
}

// FRawFileImage GetFileIcon(const std::filesystem::path& path) {
//     FRawFileImage result = {};
//     result.PathU8 = path.string();
//     result.Index = GetFileIconIndex(path);

//     std::wstring pathW = path.wstring();
//     std::replace(pathW.begin(), pathW.end(), L'/', L'\\');

//     // [Note] Try ImageFactory first for folders to get "live" content previews
//     HBITMAP hBitmap = GetFileThumbnailBITMAP(pathW, 256);

//     if (hBitmap) {
//         GetRawFileImageFromHBitmap(result, hBitmap);
//         DeleteObject(hBitmap);
//     }
//     // [Note] Fallback to traditional Shell Icon if it's a file or ImageFactory failed
//     else {
//         SHFILEINFOW fileInfo = GetFileInfo(path);
//         ICONINFO iconInfo = { 0 };
//         if (GetIconInfo(fileInfo.hIcon, &iconInfo)) {
//             if (iconInfo.hbmColor) {
//                 GetRawFileImageFromHBitmap(result, iconInfo.hbmColor);
//                 DeleteObject(iconInfo.hbmColor);
//             }
//             if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
//         }
//         DestroyIcon(fileInfo.hIcon);
//     }

//     return result;
// }

FRawFileImage GetFileThumbnail(const std::filesystem::path& path, int size) {
    FRawFileImage result = {};
    result.PathU8        = path.string();

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
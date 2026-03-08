#include "MacFile.h"

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <mach-o/dyld.h>

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() {
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
        return std::filesystem::path(buffer).lexically_normal();
    else {
        std::string path(size, '\0');
        _NSGetExecutablePath(path.data(), &size);
        return std::filesystem::path(path).lexically_normal();
    }
}

FRawIcon GetIcon(const std::filesystem::path& path) {
    FRawIcon result;
    result.Format = 1;
    result.PathU8 = path.string();

    @autoreleasepool {
        NSString* nspath = [NSString stringWithUTF8String:result.PathU8.c_str()];
        NSWorkspace* ws = [NSWorkspace sharedWorkspace];
        NSImage* iconImage = nil;

        BOOL isDirectory = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nspath isDirectory:&isDirectory];

        if (!exists) {
            result.Data = nullptr;
            result.Width = 0;
            result.Height = 0;
            result.Indice = -1;
            return result;
        }

        // Get file attributes to determine type
        NSDictionary* attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:nspath error:nil];
        NSString* fileType = attributes[NSFileType];
        
        // Determine if directory (using multiple methods for accuracy)
        BOOL isDir = isDirectory;  // From fileExistsAtPath
        if ([fileType isEqualToString:NSFileTypeDirectory]) {
            isDir = YES;
        }

        // Get icon
        if (isDir) {
            iconImage = [ws iconForFile:nspath];
        } else {
            NSString* ext = [nspath pathExtension];
            
            if (@available(macOS 11.0, *)) {
                UTType* contentType = [UTType typeWithFilenameExtension:ext];
                iconImage = [ws iconForContentType:contentType ?: UTTypeData];
            } else {
                // Fallback for older macOS versions (pre-11.0)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                iconImage = [ws iconForFileType:ext];
#pragma clang diagnostic pop
            }
        }

        if (iconImage) {
            CGFloat screenScale = [[NSScreen mainScreen] backingScaleFactor];
            NSSize targetSize = NSMakeSize(128, 128);

            NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes:NULL
                pixelsWide:targetSize.width
                pixelsHigh:targetSize.height
                bitsPerSample:8
                samplesPerPixel:4
                hasAlpha:YES
                isPlanar:NO
                colorSpaceName:NSDeviceRGBColorSpace
                bytesPerRow:targetSize.width * 4
                bitsPerPixel:32];

            [NSGraphicsContext saveGraphicsState];
            NSGraphicsContext* context = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
            context.imageInterpolation = NSImageInterpolationHigh;
            [context setShouldAntialias:YES];
            [NSGraphicsContext setCurrentContext:context];

            [iconImage drawInRect:NSMakeRect(0, 0, targetSize.width, targetSize.height)
                         fromRect:NSZeroRect
                        operation:NSCompositingOperationCopy
                         fraction:1.0];

            [NSGraphicsContext restoreGraphicsState];

            if (rep) {
                int byteSize = (int)targetSize.width * (int)targetSize.height * 4;
                uint8_t* data = (uint8_t*)malloc(byteSize);
                memcpy(data, [rep bitmapData], byteSize);

                result.Data = data;
                result.Width = (int)targetSize.width;
                result.Height = (int)targetSize.height;

                // Get iconID
                // 1000-1999: System Folders (Desktop, Documents, etc..)
                // 2000-2999: Hidden Folders (allocated by path)
                // 3000-3999: Regular Folders (allocated by path)
                // 4000-4999: Files without extension
                // 5000-5999: Reserved
                // 6000-8999: Files with extensions (allocated by extension)
                // 9000-9999: .app bundles (allocated by path)
                int iconID = 0;

                static std::unordered_map<std::string, int> s_PathToIconIDs, s_NameToIconIDs, s_SystemFolderIDs,
                    s_SpecialFolderIDs, s_ExtensionToIconIDs, s_AppPathToIconIDs;
                static std::atomic<int> s_NextID(5000), s_NextFileID(6000), s_NextAppID(9000);
                static std::once_flag s_InitFlag;

                std::call_once(s_InitFlag, []() {
                    // System root folders
                    s_SystemFolderIDs = {
                        {"/Applications", 1004},
                        {"/System", 1006},
                        {"/Library", 1005},
                        {"/Users", 2000},
                        {"/Volumes", 2001},
                        {"/Network", 2002}
                    };
                    
                    // User folder names (by name, not path)
                    s_NameToIconIDs = {
                        {"Desktop", 1001},
                        {"Documents", 1002},
                        {"Downloads", 1003},
                        {"Movies", 1004},
                        {"Music", 1005},
                        {"Pictures", 1006},
                        {"Public", 1007},
                        {"Applications", 1004}  // Reuse Applications icon
                    };
                    
                    // Special folders by name
                    s_SpecialFolderIDs = {
                        {".git", 1101},
                        {"node_modules", 1102},
                        {"build", 1103},
                        {"dist", 1104},
                        {"bin", 1105},
                        {"obj", 1106},
                        {"packages", 1107},
                        {"vendor", 1108},
                        {".vscode", 1201},
                        {".idea", 1202},
                        {".vs", 1203},
                        {"__pycache__", 1204},
                        {"assets", 1301},
                        {"resources", 1302},
                        {"images", 1303},
                        {"fonts", 1304},
                        {"sounds", 1305},
                        {"videos", 1306},
                        {"config", 1401},
                        {"settings", 1402},
                        {"data", 1403},
                        {"logs", 1404},
                        {"temp", 1405},
                        {"cache", 1406},
                        {"docs", 1501},
                        {"notes", 1502},
                        {"archive", 1503},
                        {"backup", 1504},
                        {"src", 1601},
                        {"source", 1602},
                        {"include", 1603},
                        {"lib", 1604},
                        {"test", 1605},
                        {"tests", 1606},
                        {"examples", 1607},
                        {"samples", 1608}
                    };
                });

                std::string folderName = [nspath lastPathComponent].UTF8String;
                std::string fullPath = nspath.UTF8String;

                // ===== Priority 1: Check if it's a .app bundle =====
                if ([[nspath pathExtension].lowercaseString isEqualToString:@"app"]) {
                    auto it = s_AppPathToIconIDs.find(fullPath);
                    if (it != s_AppPathToIconIDs.end()) {
                        iconID = it->second;
                    } else {
                        iconID = s_NextAppID.fetch_add(1);
                        s_AppPathToIconIDs[fullPath] = iconID;
                    }
                }
                // ===== Priority 2: Check system root folders (by path) =====
                else if (s_SystemFolderIDs.find(fullPath) != s_SystemFolderIDs.end()) {
                    iconID = s_SystemFolderIDs[fullPath];
                }
                // ===== Priority 3: Check special folders (by name) =====
                else if (s_SpecialFolderIDs.find(folderName) != s_SpecialFolderIDs.end()) {
                    iconID = s_SpecialFolderIDs[folderName];
                }
                // ===== Priority 4: Check user system folders (by name) =====
                else if (s_NameToIconIDs.find(folderName) != s_NameToIconIDs.end()) {
                    iconID = s_NameToIconIDs[folderName];
                }
                // ===== Priority 5: Folder handling =====
                else if (isDir) {
                    // Hidden folders
                    if (!folderName.empty() && folderName[0] == '.') {
                        auto cached = s_PathToIconIDs.find(fullPath);
                        if (cached != s_PathToIconIDs.end()) {
                            iconID = cached->second;
                        } else {
                            iconID = s_NextID.fetch_add(1);
                            s_PathToIconIDs[fullPath] = iconID;
                        }
                    }
                    // Regular folders
                    else {
                        auto cached = s_PathToIconIDs.find(fullPath);
                        if (cached != s_PathToIconIDs.end()) {
                            iconID = cached->second;
                        } else {
                            iconID = s_NextID.fetch_add(1);
                            s_PathToIconIDs[fullPath] = iconID;
                        }
                    }
                }
                // ===== Priority 6: File handling =====
                else {
                    std::string ext = [nspath pathExtension].lowercaseString.UTF8String;
                    if (!ext.empty()) {
                        auto it = s_ExtensionToIconIDs.find(ext);
                        if (it != s_ExtensionToIconIDs.end()) {
                            iconID = it->second;
                        } else {
                            iconID = s_NextFileID.fetch_add(1);
                            s_ExtensionToIconIDs[ext] = iconID;
                        }
                    } else {
                        iconID = 4000;
                    }
                }

                result.Indice = iconID;
            }
        }
    }
    return result;
}

bool IsHiddenOrSystem(const std::filesystem::path& p) {
    std::string filename = p.filename().string();

    if (!filename.empty() && filename[0] == '.') {
        return true;
    }
    
    const std::vector<std::string> macSystemFiles = {
        ".DS_Store",
        ".localized",
        ".Spotlight-V100",
        ".Trashes",
        ".fseventsd",
        ".TemporaryItems",
        "Desktop DB",
        "Desktop DF",
        "$RECYCLE.BIN",
    };
    
    for (const auto& file : macSystemFiles) {
        if (filename == file) {
            return true;
        }
    }

    return false;
}

} // namespace ChozoUtils::File
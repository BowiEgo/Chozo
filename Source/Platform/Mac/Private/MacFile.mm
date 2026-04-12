#include "MacFile.h"

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <QuickLookThumbnailing/QuickLookThumbnailing.h>
#import <CoreServices/CoreServices.h>

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

// 1000-1999: System Folders (Desktop, Documents, etc..)
// 2000-2999: Hidden Folders (allocated by path)
// 3000-3999: Regular Folders (allocated by path)
// 4000-4999: Files without extension
// 5000-5999: Reserved
// 6000-8999: Files with extensions (allocated by extension)
// 9000-9999: .app bundles (allocated by path)
int GetFileIconIndex(const std::filesystem::path& path) {
    int iconIndex = 0;

    @autoreleasepool {
        auto pathString = path.string();
        NSString* nspath = [NSString stringWithUTF8String:pathString.c_str()];

        BOOL isDirectory = NO;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:nspath isDirectory:&isDirectory];

        if (!exists) {
            return -1;
        }

        NSDictionary* attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:nspath error:nil];
        NSString* fileType = attributes[NSFileType];
        BOOL isDir = isDirectory;
        if ([fileType isEqualToString:NSFileTypeDirectory]) {
            isDir = YES;
        }

        static std::unordered_map<std::string, int> s_PathToIconIndices, s_NameToIconIndices, s_SystemFolderIDs,
            s_SpecialFolderIDs, s_ExtensionToIconIndices, s_AppPathToIconIndices;
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
            s_NameToIconIndices = {
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
            auto it = s_AppPathToIconIndices.find(fullPath);
            if (it != s_AppPathToIconIndices.end()) {
                iconIndex = it->second;
            } else {
                iconIndex = s_NextAppID.fetch_add(1);
                s_AppPathToIconIndices[fullPath] = iconIndex;
            }
        }
        // ===== Priority 2: Check system root folders (by path) =====
        else if (s_SystemFolderIDs.find(fullPath) != s_SystemFolderIDs.end()) {
            iconIndex = s_SystemFolderIDs[fullPath];
        }
        // ===== Priority 3: Check special folders (by name) =====
        else if (s_SpecialFolderIDs.find(folderName) != s_SpecialFolderIDs.end()) {
            iconIndex = s_SpecialFolderIDs[folderName];
        }
        // ===== Priority 4: Check user system folders (by name) =====
        else if (s_NameToIconIndices.find(folderName) != s_NameToIconIndices.end()) {
            iconIndex = s_NameToIconIndices[folderName];
        }
        // ===== Priority 5: Folder handling =====
        else if (isDir) {
            // Hidden folders
            if (!folderName.empty() && folderName[0] == '.') {
                auto cached = s_PathToIconIndices.find(fullPath);
                if (cached != s_PathToIconIndices.end()) {
                    iconIndex = cached->second;
                } else {
                    iconIndex = s_NextID.fetch_add(1);
                    s_PathToIconIndices[fullPath] = iconIndex;
                }
            }
            // Regular folders
            else {
                auto cached = s_PathToIconIndices.find(fullPath);
                if (cached != s_PathToIconIndices.end()) {
                    iconIndex = cached->second;
                } else {
                    iconIndex = s_NextID.fetch_add(1);
                    s_PathToIconIndices[fullPath] = iconIndex;
                }
            }
        }
        // ===== Priority 6: File handling =====
        else {
            std::string ext = [nspath pathExtension].lowercaseString.UTF8String;
            if (!ext.empty()) {
                auto it = s_ExtensionToIconIndices.find(ext);
                if (it != s_ExtensionToIconIndices.end()) {
                    iconIndex = it->second;
                } else {
                    iconIndex = s_NextFileID.fetch_add(1);
                    s_ExtensionToIconIndices[ext] = iconIndex;
                }
            } else {
                iconIndex = 4000;
            }
        }
    }

    return iconIndex;
}

FRawFileImage GetFileIcon(const std::filesystem::path& path) {
    FRawFileImage result;
    result.Format = 1;
    result.PathU8 = path.string();
    result.Index = GetFileIconIndex(path);

    if (result.Index == -1) {
        result.Data = nullptr;
        result.Width = result.Height = 0;
        return result;
    }

    @autoreleasepool {
        NSString* nspath = [NSString stringWithUTF8String:result.PathU8.c_str()];
        NSURL* fileURL = [NSURL fileURLWithPath:nspath];
        
        BOOL isDirectory = NO;
        [[NSFileManager defaultManager] fileExistsAtPath:nspath isDirectory:&isDirectory];
        BOOL isAppBundle = [[nspath pathExtension].lowercaseString isEqualToString:@"app"];

        int targetDim = 64;
        CGFloat scale = [[NSScreen mainScreen] backingScaleFactor];
        __block CGImageRef finalCGImage = NULL;

        // ===== 1. 获取 CGImage 源 =====
        // 只有当它是真正的文件夹（且不是 .app）时，才尝试 QuickLook 内容预览
        if (isDirectory && !isAppBundle) {
            dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
            QLThumbnailGenerationRequest* request = [[QLThumbnailGenerationRequest alloc] 
                initWithFileAtURL:fileURL
                size:CGSizeMake(targetDim, targetDim)
                scale:scale
                representationTypes:QLThumbnailGenerationRequestRepresentationTypeAll];
            
            [[QLThumbnailGenerator sharedGenerator] generateBestRepresentationForRequest:request
                completionHandler:^(QLThumbnailRepresentation* rep, NSError* error) {
                    if (rep && !error) finalCGImage = CGImageRetain(rep.CGImage);
                    dispatch_semaphore_signal(semaphore);
                }];
            dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC));
        }

        // 如果 QuickLook 没拿到，或者是文件/App，则使用 NSWorkspace 的标准图标
        if (!finalCGImage) {
            NSImage* iconImage = [[NSWorkspace sharedWorkspace] iconForFile:nspath];
            if (iconImage) {
                NSRect targetRect = NSMakeRect(0, 0, targetDim, targetDim);
                finalCGImage = [iconImage CGImageForProposedRect:&targetRect context:nil hints:nil];
                if (finalCGImage) CGImageRetain(finalCGImage);
            }
        }

        // ===== 2. 将 CGImage 转换为 Raw Data (BGRA) =====
        if (finalCGImage) {
            size_t width = CGImageGetWidth(finalCGImage);
            size_t height = CGImageGetHeight(finalCGImage);
            
            NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes:NULL
                pixelsWide:width
                pixelsHigh:height
                bitsPerSample:8
                samplesPerPixel:4
                hasAlpha:YES
                isPlanar:NO
                colorSpaceName:NSDeviceRGBColorSpace
                bytesPerRow:width * 4
                bitsPerPixel:32];

            [NSGraphicsContext saveGraphicsState];
            [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:rep]];
            CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
            CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), finalCGImage);
            [NSGraphicsContext restoreGraphicsState];

            if (rep) {
                int byteSize = (int)width * (int)height * 4;
                uint8_t* data = (uint8_t*)malloc(byteSize);
                memcpy(data, [rep bitmapData], byteSize);

                result.Data = data;
                result.Width = (int)width;
                result.Height = (int)height;
            }
            CGImageRelease(finalCGImage);
        }
    }
    return result;
}

FRawFileImage GetFileThumbnail(const std::filesystem::path& path, int size) {
    FRawFileImage result;
    result.PathU8 = path.string();
    result.Format = 1;
    
    @autoreleasepool {
        NSURL* fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path.string().c_str()]];
        
        QLThumbnailGenerationRequest* request = [[QLThumbnailGenerationRequest alloc] 
            initWithFileAtURL:fileURL
                        size:CGSizeMake(size, size)
                        scale:[[NSScreen mainScreen] backingScaleFactor]
            representationTypes:QLThumbnailGenerationRequestRepresentationTypeAll];
        
        __block QLThumbnailRepresentation* thumbnail = nil;
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        
        QLThumbnailGenerator* generator = [QLThumbnailGenerator sharedGenerator];
        [generator generateBestRepresentationForRequest:request
            completionHandler:^(QLThumbnailRepresentation * _Nullable representation, NSError * _Nullable error) {
                if (representation && !error) {
                    thumbnail = representation;
                } else {
                    // CZ_LOG(LogUIUtils, Error, "Failed to generate thumbnail: {}", 
                    //        error.localizedDescription.UTF8String);
                }
                dispatch_semaphore_signal(semaphore);
            }];
        
        dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
        
        if (thumbnail) {
            CGImageRef cgImage = thumbnail.CGImage;
            if (cgImage) {
                size_t width = CGImageGetWidth(cgImage);
                size_t height = CGImageGetHeight(cgImage);
                
                NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
                    initWithBitmapDataPlanes:NULL
                    pixelsWide:width
                    pixelsHigh:height
                    bitsPerSample:8
                    samplesPerPixel:4
                    hasAlpha:YES
                    isPlanar:NO
                    colorSpaceName:NSDeviceRGBColorSpace
                    bytesPerRow:width * 4
                    bitsPerPixel:32];
                
                [NSGraphicsContext saveGraphicsState];
                NSGraphicsContext* context = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
                [NSGraphicsContext setCurrentContext:context];
                
                CGContextDrawImage(context.CGContext, CGRectMake(0, 0, width, height), cgImage);
                
                [NSGraphicsContext restoreGraphicsState];
                
                if (rep) {
                    int byteSize = (int)width * (int)height * 4;
                    uint8_t* data = (uint8_t*)malloc(byteSize);
                    memcpy(data, [rep bitmapData], byteSize);
                    
                    result.Data = data;
                    result.Width = (int)width;
                    result.Height = (int)height;
                    result.Index = (int)[[fileURL path] hash];
                }
            }
        }
        
        if (!result.Data) {
            return GetFileIcon(path);
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
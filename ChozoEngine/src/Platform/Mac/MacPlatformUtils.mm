#include <Cocoa/Cocoa.h>
#include <string>

std::string OpenFileDialog() {
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];

        if ([panel runModal] == NSModalResponseOK) {
            NSURL* fileUrl = [panel URL];
            return std::string([[fileUrl path] UTF8String]);
        }
    }
    return std::string();
}

std::string SaveFileDialog() {
    @autoreleasepool {
        NSSavePanel* savePanel = [NSSavePanel savePanel];
        savePanel.title = @"Save your file";
        savePanel.showsTagField = YES;
        savePanel.canCreateDirectories = YES;

        NSArray<UTType*>* contentTypes = @[];
        savePanel.allowedContentTypes = contentTypes; // Specify allowed file types

        if ([savePanel runModal] == NSModalResponseOK) {
            NSURL* result = [savePanel URL];
            if (result) {
                return std::string([[result path] UTF8String]);
            }
        }
    }
    return "";
}


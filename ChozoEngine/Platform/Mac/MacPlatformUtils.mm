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
            return {[[fileUrl path] UTF8String]};
        }
    }
    return {};
}

std::string SaveFileDialog(const std::string& defaultFileName) {
    @autoreleasepool {
        NSSavePanel* savePanel = [NSSavePanel savePanel];
        savePanel.title = @"Save your file";
        savePanel.showsTagField = YES;
        savePanel.canCreateDirectories = YES;

        NSArray<UTType*>* contentTypes = @[];
        savePanel.allowedContentTypes = contentTypes; // Specify allowed file types

        if (!defaultFileName.empty()) {
            savePanel.nameFieldStringValue = [NSString stringWithUTF8String:defaultFileName.c_str()];
        }

        if ([savePanel runModal] == NSModalResponseOK) {
            NSURL* result = [savePanel URL];
            if (result) {
                return {[[result path] UTF8String]};
            }
        }
    }
    return "";
}


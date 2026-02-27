#import "MacUtils.h"
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

namespace ChozoUtils::Mac {

    void* GetMetalLayerFromNSWindow(void* nsWindow) {
        // [Note] Use __bridge to cast void* to Objective-C object without ownership change
        NSWindow* window = (__bridge NSWindow*)nsWindow;
        NSView* view = [window contentView];

        if (![view.layer isKindOfClass:[CAMetalLayer class]]) {
            view.wantsLayer = YES;
            view.layer = [CAMetalLayer layer];
        }

        // [Note] Use __bridge to cast Objective-C object back to void*
        return (__bridge void*)view.layer;
    }

}
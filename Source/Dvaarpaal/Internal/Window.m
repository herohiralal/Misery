#include "Dvaarpaal/Window.h"
#include "Dvaarpaal/Input.h"

static b8 G_DVRPL_Internal_OSXAppInitialised = false;

DVRPL_WindowData DVRPL_CreateWindow(DVRPL_WindowCreationOptions options)
{
    @autoreleasepool {
        NSApplication* app = (__bridge NSApplication*) DVRPL_BREAK_APP_HANDLE(options.app);
        if (!G_DVRPL_Internal_OSXAppInitialised)
        {
            NSMenu* mainMenu = [[NSMenu alloc] init];
            [app setMainMenu:mainMenu];

            // Create Apple menu
            NSMenuItem* appleItem = [[NSMenuItem alloc] init];
            [mainMenu addItem:appleItem];

            NSMenu* appleMenu = [[NSMenu alloc] initWithTitle:@""];
            [appleItem setSubmenu:appleMenu];

            // Add an item to the Apple menu
            NSMenuItem* aboutItem =
                [[NSMenuItem alloc] initWithTitle:@"About This App"
                                           action:@selector(orderFrontStandardAboutPanel:)
                                    keyEquivalent:@""];
            [appleMenu addItem:aboutItem];

            NSMenuItem* fullScreenItem =
                [[NSMenuItem alloc] initWithTitle:@"Toggle Full Screen"
                                           action:@selector(toggleFullScreen:)
                                    keyEquivalent:@"f"];
            [fullScreenItem setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];
            [appleMenu addItem:fullScreenItem];

            // Add standard "Quit" item
            NSMenuItem* quitItem =
                [[NSMenuItem alloc] initWithTitle:@"Quit"
                                           action:@selector(terminate:)
                                    keyEquivalent:@"q"];
            [appleMenu addItem:quitItem];
            [app finishLaunching];

            DVRPL_Internal_AppleSetApp(app);
            G_DVRPL_Internal_OSXAppInitialised = true;
        }

        NSScreen* targetScreen = [NSScreen mainScreen];
        CGFloat pixelsToPointsFactor = ((CGFloat) 1) / [targetScreen backingScaleFactor];
        NSRect tgtFrame = {0};
        tgtFrame.origin = (CGPoint)
        {
            .x = ((CGFloat) options.posX) * pixelsToPointsFactor,
            .y = ((CGFloat) options.posY) * pixelsToPointsFactor
        };
        tgtFrame.size = (CGSize)
        {
            .width = ((CGFloat) options.sizeX) * pixelsToPointsFactor,
            .height = ((CGFloat) options.sizeY) * pixelsToPointsFactor
        };

        NSWindowStyleMask style =
            NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable |
            NSWindowStyleMaskResizable |
            NSWindowStyleMaskMiniaturizable;

        NSWindow* nativeWindow = [[NSWindow alloc] initWithContentRect:tgtFrame
                                                              styleMask:style
                                                                backing:NSBackingStoreBuffered
                                                                  defer:NO
                                                                 screen:targetScreen];


        cstring titleStr = PNSLR_CStringFromString(options.title, PNSLR_GetAllocator_DefaultHeap());
        NSString* titleNsStr = [NSString stringWithUTF8String:titleStr];
        [nativeWindow setTitle:titleNsStr];
        [nativeWindow makeKeyAndOrderFront:nil];

        if (options.posX == 0 && options.posY == 0) // uninitialised
        {
            [nativeWindow center];
        }

        NSView* view = [nativeWindow contentView];
        [view setWantsLayer:YES];

        view.layer.backgroundColor = [[NSColor colorWithCalibratedRed:((CGFloat) options.bgColR) / 255.0
                                                                 green:((CGFloat) options.bgColG) / 255.0
                                                                  blue:((CGFloat) options.bgColB) / 255.0
                                                                 alpha:((CGFloat) options.bgColA) / 255.0] CGColor];

        PNSLR_FreeCString(titleStr, PNSLR_GetAllocator_DefaultHeap(), PNSLR_GET_LOC(), nil);

        DVRPL_WindowData windowData = {0};
        windowData.window = DVRPL_MAKE_WINDOW_HANDLE((__bridge_retained rawptr) nativeWindow);

        return windowData;
    }
}

void DVRPL_DestroyWindow(DVRPL_WindowData* window)
{
    @autoreleasepool {
        NSWindow* wnd = (__bridge_transfer NSWindow*) DVRPL_BREAK_WINDOW_HANDLE(window->window);
        [wnd orderOut:nil];
        [wnd close];
        window->window = DVRPL_MAKE_WINDOW_HANDLE(DVRPL_Internal_InvalidWindowHandle);
    }
}

b8 DVRPL_SetFullScreen(DVRPL_WindowData* window, b8 status, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    @autoreleasepool {
        DVRPL_Internal_NativeSavedWindowData savedData = DVRPL_BREAK_SAVED_WINDOW_DATA(window->savedData);
        NSApplication* app = (__bridge NSApplication*) DVRPL_BREAK_APP_HANDLE(savedData.owningApp);
        NSWindow* wnd = (__bridge NSWindow*) DVRPL_BREAK_WINDOW_HANDLE(window->window);
        NSView* view = [wnd contentView];
        CGRect frame = [view frame];
        NSWindowStyleMask mask = [wnd styleMask];

        if ((mask & NSWindowStyleMaskFullScreen) && status) {
            if (posX) *posX = (i16)frame.origin.x;
            if (posY) *posY = (i16)frame.origin.y;
            if (sizeX) *sizeX = (u16)frame.size.width;
            if (sizeY) *sizeY = (u16)frame.size.height;
            return true;
        }

        [wnd toggleFullScreen:nil];

        if (status)
            [app setPresentationOptions:(NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar)];
        else
            [app setPresentationOptions:NSApplicationPresentationDefault];

        frame = [view frame];
        NSWindowStyleMask mask2 = [wnd styleMask];

        if (posX) *posX = (i16)frame.origin.x;
        if (posY) *posY = (i16)frame.origin.y;
        if (sizeX) *sizeX = (u16)frame.size.width;
        if (sizeY) *sizeY = (u16)frame.size.height;

        return (mask2 & NSWindowStyleMaskFullScreen) != (mask & NSWindowStyleMaskFullScreen);
    }
}


b8 DVRPL_GetWindowDimensions(DVRPL_WindowData* window, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    @autoreleasepool {
        if (!window || DVRPL_BREAK_WINDOW_HANDLE(window->window) == DVRPL_Internal_InvalidWindowHandle)
            return false;

        NSWindow* wnd = (__bridge NSWindow*) DVRPL_BREAK_WINDOW_HANDLE(window->window);
        NSRect frame = [wnd frame];

        i16 x = (i16) frame.origin.x;
        i16 y = (i16) frame.origin.y;
        u16 w = (u16) frame.size.width;
        u16 h = (u16) frame.size.height;

        if (posX)  *posX  = x;
        if (posY)  *posY  = y;
        if (sizeX) *sizeX = w;
        if (sizeY) *sizeY = h;
        return true;
    }
}

b8 DVRPL_GetPtrPosFromWindow(DVRPL_Window window, i16* posX, i16* posY)
{
    @autoreleasepool {
        if (DVRPL_BREAK_WINDOW_HANDLE(window) == DVRPL_Internal_InvalidWindowHandle)
            return false;

        NSWindow* wnd = (__bridge NSWindow*) DVRPL_BREAK_WINDOW_HANDLE(window);
        NSPoint mouseLocation = [wnd mouseLocationOutsideOfEventStream];

        NSRect frame = [wnd contentRectForFrameRect:[wnd frame]];
        // Flip Y: 0 = top
        i16 x = (i16) mouseLocation.x;
        i16 y = (i16)(frame.size.height - mouseLocation.y);

        if (posX) *posX = x;
        if (posY) *posY = y;
        return true;
    }
}

b8 DVRPL_GetPtrPos(i16* posX, i16* posY)
{
    @autoreleasepool {
        NSPoint mouseLocation = [NSEvent mouseLocation];

        NSScreen* mainScreen = [NSScreen mainScreen];
        CGFloat screenHeight = mainScreen.frame.size.height;

        i16 x = (i16) mouseLocation.x;
        i16 y = (i16)(screenHeight - mouseLocation.y);

        if (posX) *posX = x;
        if (posY) *posY = y;
        return true;
    }
}


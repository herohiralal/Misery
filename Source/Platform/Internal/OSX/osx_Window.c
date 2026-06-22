#include "osx_Platform.h"

#if MSR_OSX

static b8 G_WND_Internal_AppInitialised = false;

static void WND_Internal_InitialiseApp(APP_Handle app)
{
    if (G_WND_Internal_AppInitialised)
        return;

    NSApplication* nsApp = (NSApplication*) APP_FromHandle(app);
    if (!nsApp)
        nsApp = [NSApplication sharedApplication];

    [nsApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu* mainMenu = [[NSMenu alloc] init];
    [nsApp setMainMenu:mainMenu];

    NSMenuItem* appleItem = [[NSMenuItem alloc] init];
    [mainMenu addItem:appleItem];

    NSMenu* appleMenu = [[NSMenu alloc] initWithTitle:@""];
    [appleItem setSubmenu:appleMenu];

    NSMenuItem* aboutItem = [[NSMenuItem alloc] initWithTitle:@"About Misery"
                                                       action:@selector(orderFrontStandardAboutPanel:)
                                                keyEquivalent:@""];
    [appleMenu addItem:aboutItem];
    [aboutItem release];

    NSMenuItem* fullScreenItem = [[NSMenuItem alloc] initWithTitle:@"Toggle Full Screen"
                                                             action:@selector(toggleFullScreen:)
                                                      keyEquivalent:@"f"];
    [fullScreenItem setKeyEquivalentModifierMask:(NSEventModifierFlagControl | NSEventModifierFlagCommand)];
    [appleMenu addItem:fullScreenItem];
    [fullScreenItem release];

    NSMenuItem* quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                      action:@selector(terminate:)
                                               keyEquivalent:@"q"];
    [appleMenu addItem:quitItem];
    [quitItem release];

    [appleMenu release];
    [appleItem release];
    [mainMenu release];

    [nsApp finishLaunching];
    [nsApp activateIgnoringOtherApps:YES];

    G_WND_Internal_AppInitialised = true;
}

WND_Data WND_Create(WND_Cfg cfg)
{
    @autoreleasepool
    {
        WND_Internal_InitialiseApp(cfg.app);

        NSApplication* nsApp = (NSApplication*) APP_FromHandle(cfg.app);
        if (!nsApp)
            nsApp = [NSApplication sharedApplication];

        NSScreen* targetScreen = [NSScreen mainScreen];
        if (!targetScreen)
            targetScreen = [[NSScreen screens] firstObject];

        NSRect contentRect = NSMakeRect(
            (CGFloat) cfg.posX,
            (CGFloat) cfg.posY,
            (CGFloat) (cfg.sizeX ? cfg.sizeX : 800),
            (CGFloat) (cfg.sizeY ? cfg.sizeY : 600)
        );

        NSWindowStyleMask style =
            NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable |
            NSWindowStyleMaskResizable |
            NSWindowStyleMaskMiniaturizable;

        NSWindow* nativeWindow = [[NSWindow alloc] initWithContentRect:contentRect
                                                              styleMask:style
                                                                backing:NSBackingStoreBuffered
                                                                  defer:NO
                                                                 screen:targetScreen];

        if (!nativeWindow)
            return (WND_Data) {0};

        [nativeWindow setReleasedWhenClosed:NO];
        [nativeWindow setAcceptsMouseMovedEvents:YES];

        if (cfg.title.data && cfg.title.count > 0)
        {
            cstring title = STR_CloneToCStr(cfg.title, MEM_temp);
            NSString* titleNs = [NSString stringWithUTF8String:title ? title : ""];
            [nativeWindow setTitle:titleNs ? titleNs : @""];
        }

        NSColor* bg = [NSColor colorWithCalibratedRed:((CGFloat) cfg.bgCol[0]) / 255.0
                                                green:((CGFloat) cfg.bgCol[1]) / 255.0
                                                 blue:((CGFloat) cfg.bgCol[2]) / 255.0
                                                alpha:((CGFloat) cfg.bgCol[3]) / 255.0];
        [nativeWindow setBackgroundColor:bg];

        if (cfg.posX <= 0 && cfg.posY <= 0)
            [nativeWindow center];

        INP_Internal_OSXAttachWindowDelegate(nativeWindow);

        [nativeWindow makeKeyAndOrderFront:nil];
        [nsApp updateWindows];

        NSRect frame = [nativeWindow frame];
        WND_NativeSavedData saved =
        {
            .x = frame.origin.x,
            .y = frame.origin.y,
            .w = frame.size.width,
            .h = frame.size.height,
            .wasFullscreen = 0,
        };

        return (WND_Data)
        {
            .handle = WND_ToHandle((rawptr) nativeWindow),
            .savedData = WND_FromSavedData(saved),
        };
    }
}

void WND_Destroy(WND_Data* window)
{
    if (!window)
        return;

    NSWindow* wnd = (NSWindow*) WND_FromHandle(window->handle);
    if (!wnd)
        return;

    [wnd orderOut:nil];
    [wnd close];
    [wnd release];

    window->handle = (WND_Handle) {0};
}

b8 WND_SetFullScreen(WND_Data* window, b8 status, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (!window)
        return false;

    NSWindow* wnd = (NSWindow*) WND_FromHandle(window->handle);
    if (!wnd)
        return false;

    NSWindowStyleMask currMask = [wnd styleMask];
    b8 isFullscreen = !!(currMask & NSWindowStyleMaskFullScreen);

    if (status && !isFullscreen)
    {
        NSRect frame = [wnd frame];
        window->savedData = WND_FromSavedData((WND_NativeSavedData)
        {
            .x = frame.origin.x,
            .y = frame.origin.y,
            .w = frame.size.width,
            .h = frame.size.height,
            .wasFullscreen = 1,
        });

        [wnd toggleFullScreen:nil];
        [NSApp setPresentationOptions:(NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar)];
    }
    else if (!status && isFullscreen)
    {
        [wnd toggleFullScreen:nil];
        [NSApp setPresentationOptions:NSApplicationPresentationDefault];

        WND_NativeSavedData saved = WND_ToSavedData(window->savedData);
        if (saved.wasFullscreen)
        {
            NSRect target = NSMakeRect((CGFloat) saved.x, (CGFloat) saved.y, (CGFloat) saved.w, (CGFloat) saved.h);
            [wnd setFrame:target display:YES];
        }
    }

    i16 x = 0, y = 0;
    u16 w = 0, h = 0;
    if (!WND_GetDimensions(window, &x, &y, &w, &h))
        return false;

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;

    return true;
}

b8 WND_GetDimensions(WND_Data* window, i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (!window)
        return false;

    NSWindow* wnd = (NSWindow*) WND_FromHandle(window->handle);
    if (!wnd)
        return false;

    NSRect frame = [wnd frame];
    NSRect contentRect = [wnd contentRectForFrameRect:frame];

    i16 x = (i16) frame.origin.x;
    i16 y = (i16) frame.origin.y;
    u16 w = (u16) contentRect.size.width;
    u16 h = (u16) contentRect.size.height;

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;
    return true;
}

b8 WND_GetPtrPos(WND_Data* window, i16* posX, i16* posY)
{
    if (!window)
        return false;

    NSWindow* wnd = (NSWindow*) WND_FromHandle(window->handle);
    if (!wnd)
        return false;

    NSPoint p = [wnd mouseLocationOutsideOfEventStream];
    NSRect frame = [wnd frame];
    NSRect contentRect = [wnd contentRectForFrameRect:frame];

    i16 x = (i16) p.x;
    i16 y = (i16) (contentRect.size.height - p.y);

    if (posX) *posX = x;
    if (posY) *posY = y;
    return true;
}

b8 WND_Rename(WND_Data* window, utf8str newName)
{
    if (!window)
        return false;

    NSWindow* wnd = (NSWindow*) WND_FromHandle(window->handle);
    if (!wnd)
        return false;

    cstring nameCStr = STR_CloneToCStr(newName, MEM_temp);
    NSString* name = [NSString stringWithUTF8String:nameCStr ? nameCStr : ""];
    [wnd setTitle:(name ? name : @"")];
    return true;
}

#endif

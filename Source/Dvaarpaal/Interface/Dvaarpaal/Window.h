/**
 * The window module for Dvaarpaal.
 * A lot of patterns in this file are all about opaque handles.
 * The real reason for that is because of how programming in C/C++ works
 * at the core, and how it involves 'including' the files into your
 * source code.
 *
 * It's fine in a lot of cases, but given how LSPs and auto-completion are
 * valuable tools for programmers these days, it sucks to see a list of
 * irrelevant functions, that you'll probably never be calling because
 * they're specific to a platform.
 *
 * This kind of an opaque-handle approach allows us to make it so that
 * these platform-specific details only need to be included in the private
 * implementation files, while the APIs remain strictly cross-platform
 * and easy to use.
 */
#ifndef DVRPL_WINDOW_H // ==========================================================
#define DVRPL_WINDOW_H
#include "__Prelude.h"
EXTERN_C_BEGIN

/**
 * A cross-plataform opaque handle to the application instance.
 * On Windows, this is an HINSTANCE.
 * On OSX, this is an NSApplication*.
 * On Android, this is a struct android_app*.
 */
typedef struct DVRPL_App
{
    u64 handle;
} DVRPL_App;

/**
 * A cross-platform opaque handle to a window.
 * On Windows, this is an HWND.
 * On OSX, this is an NSWindow*.
 * On Android, this is an ANativeWindow*.
 */
typedef struct DVRPL_Window
{
    u64 handle;
} DVRPL_Window;

/**
 * A cross-platform opaque handle to saved information for a window.
 */
typedef struct alignas(PNSLR_PTR_SIZE) DVRPL_SavedWindowData
{
    u8 buffer[5 * PNSLR_PTR_SIZE];
} DVRPL_SavedWindowData;

/**
 * A cross-platform opaque handle for window data.
 * Includes some amount of saved data for the window.
 */
typedef struct DVRPL_WindowData
{
    DVRPL_Window          window;
    DVRPL_SavedWindowData savedData;
} DVRPL_WindowData;

/**
 * Options for creating a window.
 */
typedef struct DVRPL_WindowCreationOptions
{
    DVRPL_App    app;
    i16          posX;
    i16          posY;
    u16          sizeX;
    u16          sizeY;
    utf8str      title;
    DVRPL_Window parent;
    u8           bgColR;
    u8           bgColG;
    u8           bgColB;
    u8           bgColA;
    b8           msaa;
    b8           acceptDropFiles;
} DVRPL_WindowCreationOptions;

/**
 * Creates a window with the specified options.
 * Returns a handle to the created window.
 * If creation failed, the returned handle will be zeroed.
 * Not thread-safe.
 */
DVRPL_WindowData DVRPL_CreateWindow(DVRPL_WindowCreationOptions options);

/**
 * Destroys the specified window.
 * Not thread-safe.
 */
void DVRPL_DestroyWindow(DVRPL_WindowData* window);

/**
 * Sets the window's fullscreen status.
 * Returns true on success, false on failure.
 * If entering fullscreen, the previous position and size are stored in the provided pointers.
 * If exiting fullscreen, the window is restored to the provided position and size.
 * If posX, posY, sizeX or sizeY are null, reasonable defaults are used instead.
 * Not thread-safe.
 */
b8 DVRPL_SetFullScreen(
    DVRPL_WindowData* window,
    b8 status,
    i16* posX OPT_ARG,
    i16* posY OPT_ARG,
    u16* sizeX OPT_ARG,
    u16* sizeY OPT_ARG
);

/**
 * Gets the window's current position and size.
 * Returns true on success, false on failure.
 * If any of posX, posY, sizeX or sizeY are null, they are ignored, otherwise
 * the corresponding value is written to the provided pointer.
 * Not thread-safe.
 */
b8 DVRPL_GetWindowDimensions(DVRPL_WindowData* window, i16* posX, i16* posY, u16* sizeX, u16* sizeY);

/**
 * Get the current position of the pointer relative to the window's top-left corner.
 * Returns true on success, false on failure.
 * If posX or posY are null, they are ignored, otherwise the corresponding value is written to the provided pointer.
 * Not thread-safe.
 */
b8 DVRPL_GetPtrPosFromWindow(DVRPL_Window window, i16* posX, i16* posY);

/**
 * Get the current position of the pointer relative to the screen's top-left corner.
 * Returns true on success, false on failure.
 * If posX or posY are null, they are ignored, otherwise the corresponding value is written to the provided pointer.
 * Not thread-safe.
 */
b8 DVRPL_GetPtrPos(i16* posX, i16* posY);

/**
 * Changes the window's name, if possible to do so.
 * Returns true on success, false on failure.
 * Not thread-safe.
 */
b8 DVRPL_RenameWindow(DVRPL_Window window, utf8str newName);

//+skipreflect
#ifdef DVRPL_IMPLEMENTATION
    #if PNSLR_WINDOWS
        typedef HINSTANCE DVRPL_Internal_NativeAppHandle;
        typedef HWND DVRPL_Internal_NativeWindowHandle;
        static const DVRPL_Internal_NativeWindowHandle DVRPL_Internal_InvalidWindowHandle = NULL;

        typedef struct
        {
            RECT rect;
            LONG savedStyle;
            LONG savedExStyle;
        } DVRPL_Internal_NativeSavedWindowData;
    #elif PNSLR_OSX
        typedef rawptr DVRPL_Internal_NativeAppHandle; // NSApplication*
        typedef rawptr DVRPL_Internal_NativeWindowHandle; // NSWindow*
        static const DVRPL_Internal_NativeWindowHandle DVRPL_Internal_InvalidWindowHandle = nil;

        typedef struct
        {
            #ifdef __OBJC__
                DVRPL_App owningApp;
            #else
                u64 data; // stub to compile
            #endif
        } DVRPL_Internal_NativeSavedWindowData;
    #elif PNSLR_ANDROID
        typedef struct android_app* DVRPL_Internal_NativeAppHandle;
        typedef ANativeWindow* DVRPL_Internal_NativeWindowHandle;
        static const DVRPL_Internal_NativeWindowHandle DVRPL_Internal_InvalidWindowHandle = nil;

        typedef struct
        {
            DVRPL_Internal_NativeAppHandle app;
        } DVRPL_Internal_NativeSavedWindowData;
    #else
        #error "Unimplemented."
    #endif

    static_assert(sizeof(DVRPL_App)  == sizeof(DVRPL_Internal_NativeAppHandle),  "DVRPL_App and DVRPL_Internal_NativeAppHandle must have the same size.");
    static_assert(alignof(DVRPL_App) == alignof(DVRPL_Internal_NativeAppHandle), "DVRPL_App and DVRPL_Internal_NativeAppHandle must have the same alignment.");

    static_assert(sizeof(DVRPL_Window)  == sizeof(DVRPL_Internal_NativeWindowHandle),  "DVRPL_Window and DVRPL_Internal_NativeWindowHandle must have the same size.");
    static_assert(alignof(DVRPL_Window) == alignof(DVRPL_Internal_NativeWindowHandle), "DVRPL_Window and DVRPL_Internal_NativeWindowHandle must have the same alignment.");

    static_assert(sizeof(DVRPL_SavedWindowData)  >= sizeof(DVRPL_Internal_NativeSavedWindowData),  "DVRPL_SavedWindowData must be large   enough to hold DVRPL_Internal_NativeSavedWindowData.");
    static_assert(alignof(DVRPL_SavedWindowData) >= alignof(DVRPL_Internal_NativeSavedWindowData), "DVRPL_SavedWindowData must be aligned enough to hold DVRPL_Internal_NativeSavedWindowData.");

    static PNSLR_FORCEINLINE DVRPL_Internal_NativeAppHandle DVRPL_BREAK_APP_HANDLE(DVRPL_App h) { return *(DVRPL_Internal_NativeAppHandle*)&(h.handle); }
    static PNSLR_FORCEINLINE DVRPL_App DVRPL_MAKE_APP_HANDLE(DVRPL_Internal_NativeAppHandle h) { return *(DVRPL_App*)&(h); }

    static PNSLR_FORCEINLINE DVRPL_Internal_NativeWindowHandle DVRPL_BREAK_WINDOW_HANDLE(DVRPL_Window h) { return *(DVRPL_Internal_NativeWindowHandle*)&(h.handle); }
    static PNSLR_FORCEINLINE DVRPL_Window DVRPL_MAKE_WINDOW_HANDLE(DVRPL_Internal_NativeWindowHandle h) { return *(DVRPL_Window*)&(h); }

    static PNSLR_FORCEINLINE DVRPL_Internal_NativeSavedWindowData DVRPL_BREAK_SAVED_WINDOW_DATA(DVRPL_SavedWindowData d) { return *(DVRPL_Internal_NativeSavedWindowData*)&(d.buffer); }
    static PNSLR_FORCEINLINE DVRPL_SavedWindowData DVRPL_MAKE_SAVED_WINDOW_DATA(DVRPL_Internal_NativeSavedWindowData d)
    {
        DVRPL_SavedWindowData result = {0};
        PNSLR_MemCopy(&result, &d, sizeof(DVRPL_Internal_NativeSavedWindowData));
        return result;
    }
#endif
//-skipreflect

EXTERN_C_END
#endif // DVRPL_WINDOW_H ===========================================================

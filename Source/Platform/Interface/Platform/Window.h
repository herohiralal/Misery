#pragma once
#include <__init.h>
#include <Core/Core.h>

EXTERN_C_BEGIN

/**
 * A cross-platform opaque handle to a window.
 * On Windows, this is an HWND.
 * On OSX, this is an NSWindow*.
 * On Android, this is an ANativeWindow*.
 */
typedef struct
{
    usize handle;
} WND_Handle;

/**
 * A cross-platform opaque handle to saved information for a window.
 */
typedef struct
{
    usize buffer[5];
} WND_SavedData;

/**
 * A cross-platform opaque handle for window data.
 * Includes some amount of saved data for the window.
 */
typedef struct
{
    WND_Handle    handle;
    WND_SavedData savedData;
} WND_Data;

/**
 * Configuration for creating a window.
 */
typedef struct
{
    APP_Handle app;
    i16        posX,  posY;
    u16        sizeX, sizeY;
    utf8str    title;
    WND_Handle parent;
    u8         bgCol[4]; // rgba
    b8         msaa, acceptDropFiles;
} WND_Cfg;

/**
 * Creates a window with the specified configuration.
 * Returns the data of the created window.
 * If creation failed, the returned handle will be zeroed.
 * Not thread-safe.
 */
WND_Data WND_Create(WND_Cfg);

/**
 * Destroys the specified window.
 * Not thread-safe.
 */
void WND_Destroy(WND_Data* window);

/**
 * Sets the window's fullscreen status.
 * Returns true on success, false on failure.
 * If entering fullscreen, the previous position and size are stored in the provided pointers.
 * If exiting fullscreen, the window is restored to the provided position and size.
 * If posX, posY, sizeX or sizeY are null, reasonable defaults are used instead.
 * Not thread-safe.
 */
b8 WND_SetFullScreen(
    WND_Data* window,
    b8 status,
    i16* posX  OPT_ARG, i16* posY  OPT_ARG,
    u16* sizeX OPT_ARG, u16* sizeY OPT_ARG
);

/**
 * Gets the window's current position and size.
 * Returns true on success, false on failure.
 * If any of posX, posY, sizeX or sizeY are null, they are ignored, otherwise
 * the corresponding value is written to the provided pointer.
 * Not thread-safe.
 */
b8 WND_GetDimensions(
    WND_Data* window,
    i16* posX  OPT_ARG, i16* posY  OPT_ARG,
    u16* sizeX OPT_ARG, u16* sizeY OPT_ARG
);

/**
 * Get the current position of the pointer relative to the window's top-left corner.
 * Returns true on success, false on failure.
 * If posX or posY are null, they are ignored, otherwise the corresponding value is written to the provided pointer.
 * Not thread-safe.
 */
b8 WND_GetPtrPos(WND_Data* window, i16* posX, i16* posY);

/**
 * Changes the window's name, if possible to do so.
 * Returns true on success, false on failure.
 * Not thread-safe.
 */
b8 WND_Rename(WND_Data* window, utf8str newName);

#if MSR_WINDOWS

    struct HWND__;
    typedef struct HWND__ *HWND;

    static_assert( sizeof(WND_Handle) ==  sizeof(HWND), "window struct size mismatch");
    static_assert(alignof(WND_Handle) == alignof(HWND), "window struct alignment mismatch");

    static inline WND_Handle WND_ToHandle(HWND hInstance) { return *(WND_Handle*) &hInstance; }
    static inline HWND WND_FromHandle(WND_Handle window) { return *(HWND*) &window; }

#elif MSR_OSX
    @class NSWindow;

    static_assert( sizeof(WND_Handle) ==  sizeof(NSWindow*), "window struct size mismatch");
    static_assert(alignof(WND_Handle) == alignof(NSWindow*), "window struct alignment mismatch");

    static inline WND_Handle WND_ToHandle(NSWindow* window) { return *(WND_Handle*) &window; }
    static inline NSWindow* WND_FromHandle(WND_Handle window) { return *(NSWindow**) &window; }

#elif MSR_LINUX
#elif MSR_ANDROID

    struct ANativeWindow;
    typedef struct ANativeWindow ANativeWindow;

    static_assert( sizeof(WND_Handle) ==  sizeof(ANativeWindow*), "window struct size mismatch");
    static_assert(alignof(WND_Handle) == alignof(ANativeWindow*), "window struct alignment mismatch");

    static inline WND_Handle WND_ToHandle(ANativeWindow* window) { return *(WND_Handle*) &window; }
    static inline ANativeWindow* WND_FromHandle(WND_Handle window) { return *(ANativeWindow**) &window; }

#else
    #error "unimplemented platform"
#endif

EXTERN_C_END

#pragma once
#include <ExtDeps_Core.h>

MSR_SUPPRESS_WARN

#if MSR_LINUX
    #include <xcb/xcb.h>
    #include <xcb/xcb_icccm.h>
    #include <xcb/xinput.h>
    #include <xcb/xcb_keysyms.h>
    #include <X11/keysym.h>
#endif

MSR_UNSUPPRESS_WARN

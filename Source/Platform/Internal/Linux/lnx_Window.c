#include "lnx_Platform.h"

#if MSR_LINUX

// ─── XCB context ────────────────────────────────────────────────────────────

static WND_XCBContext G_WND_XCBCtx   = {0};
static b8             G_WND_XCBReady = false;

static xcb_atom_t WND_InternAtom(xcb_connection_t* c, const char* name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, 0, (u16) strlen(name), name);
    xcb_intern_atom_reply_t* reply  = xcb_intern_atom_reply(c, cookie, nil);
    if (!reply) return XCB_ATOM_NONE;
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

WND_XCBContext* WND_GetXCBContext(void)
{
    if (G_WND_XCBReady)
        return &G_WND_XCBCtx;

    int screenNum = 0;
    G_WND_XCBCtx.connection = xcb_connect(nil, &screenNum);
    if (!G_WND_XCBCtx.connection || xcb_connection_has_error(G_WND_XCBCtx.connection))
        return nil;

    xcb_setup_t const*    setup = xcb_get_setup(G_WND_XCBCtx.connection);
    xcb_screen_iterator_t iter  = xcb_setup_roots_iterator(setup);
    for (int i = 0; i < screenNum; i++)
        xcb_screen_next(&iter);
    G_WND_XCBCtx.screen = iter.data;

    // Intern atoms
    G_WND_XCBCtx.WM_PROTOCOLS            = WND_InternAtom(G_WND_XCBCtx.connection, "WM_PROTOCOLS");
    G_WND_XCBCtx.WM_DELETE_WINDOW        = WND_InternAtom(G_WND_XCBCtx.connection, "WM_DELETE_WINDOW");
    G_WND_XCBCtx._NET_WM_STATE           = WND_InternAtom(G_WND_XCBCtx.connection, "_NET_WM_STATE");
    G_WND_XCBCtx._NET_WM_STATE_FULLSCREEN= WND_InternAtom(G_WND_XCBCtx.connection, "_NET_WM_STATE_FULLSCREEN");
    G_WND_XCBCtx._NET_WM_NAME            = WND_InternAtom(G_WND_XCBCtx.connection, "_NET_WM_NAME");
    G_WND_XCBCtx.UTF8_STRING             = WND_InternAtom(G_WND_XCBCtx.connection, "UTF8_STRING");

    // XInput2
    xcb_query_extension_cookie_t xiCookie = xcb_query_extension(G_WND_XCBCtx.connection, 15, "XInputExtension");
    xcb_query_extension_reply_t* xiReply  = xcb_query_extension_reply(G_WND_XCBCtx.connection, xiCookie, nil);
    if (xiReply && xiReply->present)
    {
        G_WND_XCBCtx.xi2Opcode    = xiReply->major_opcode;
        G_WND_XCBCtx.xi2Available = true;
        free(xiReply);

        // Select raw mouse motion on the root window
        struct
        {
            xcb_input_event_mask_t   head;
            xcb_input_xi_event_mask_t mask;
        } evmask;
        evmask.head.deviceid = XCB_INPUT_DEVICE_ALL_MASTER;
        evmask.head.mask_len = 1;
        evmask.mask          = XCB_INPUT_XI_EVENT_MASK_RAW_MOTION;
        xcb_input_xi_select_events(G_WND_XCBCtx.connection,
                                   G_WND_XCBCtx.screen->root,
                                   1, &evmask.head);
    }
    else
    {
        if (xiReply) free(xiReply);
    }

    xcb_flush(G_WND_XCBCtx.connection);
    G_WND_XCBReady = true;
    return &G_WND_XCBCtx;
}

// ─── Window ─────────────────────────────────────────────────────────────────

WND_Data WND_Create(WND_Cfg cfg)
{
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return (WND_Data) {0};

    xcb_connection_t* c      = ctx->connection;
    xcb_screen_t*     screen = ctx->screen;

    i16 x = cfg.posX, y = cfg.posY;
    if (x <= 0 && y <= 0) { x = 15; y = 15; }

    xcb_window_t win = xcb_generate_id(c);

    u32 valueMask   = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 values[2];
    values[0] = (cfg.bgCol[0] << 16) | (cfg.bgCol[1] << 8) | cfg.bgCol[2]; // RGB packed
    values[1] =
        XCB_EVENT_MASK_KEY_PRESS         |
        XCB_EVENT_MASK_KEY_RELEASE       |
        XCB_EVENT_MASK_BUTTON_PRESS      |
        XCB_EVENT_MASK_BUTTON_RELEASE    |
        XCB_EVENT_MASK_POINTER_MOTION    |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY  |
        XCB_EVENT_MASK_FOCUS_CHANGE      |
        XCB_EVENT_MASK_EXPOSURE;

    xcb_create_window(
        c,
        XCB_COPY_FROM_PARENT,
        win,
        cfg.parent.handle ? (xcb_window_t) cfg.parent.handle : screen->root,
        x, y,
        cfg.sizeX ? cfg.sizeX : 800,
        cfg.sizeY ? cfg.sizeY : 600,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        valueMask, values
    );

    // Title
    if (cfg.title.data && cfg.title.count > 0)
    {
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, win,
            XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
            (u32) cfg.title.count, cfg.title.data);
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, win,
            ctx->_NET_WM_NAME, ctx->UTF8_STRING, 8,
            (u32) cfg.title.count, cfg.title.data);
    }

    // WM_DELETE_WINDOW protocol
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, win,
        ctx->WM_PROTOCOLS, XCB_ATOM_ATOM, 32,
        1, &ctx->WM_DELETE_WINDOW);

    xcb_map_window(c, win);
    xcb_flush(c);

    WND_NativeSavedData saved =
    {
        .posX  = x, .posY  = y,
        .sizeX = cfg.sizeX, .sizeY = cfg.sizeY,
    };

    return (WND_Data)
    {
        .handle    = (WND_Handle) { .handle = (usize) win },
        .savedData = WND_FromSavedData(saved),
    };
}

void WND_Destroy(WND_Data* window)
{
    if (!window) return;
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return;

    xcb_window_t win = (xcb_window_t) window->handle.handle;
    if (!win) return;

    xcb_destroy_window(ctx->connection, win);
    xcb_flush(ctx->connection);
    window->handle = (WND_Handle) {0};
}

b8 WND_SetFullScreen(WND_Data* window, b8 status,
    i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (!window) return false;
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return false;

    xcb_window_t win = (xcb_window_t) window->handle.handle;
    if (!win) return false;

    xcb_connection_t* c = ctx->connection;

    // Send _NET_WM_STATE_FULLSCREEN client message
    xcb_client_message_event_t ev = {0};
    ev.response_type  = XCB_CLIENT_MESSAGE;
    ev.format         = 32;
    ev.window         = win;
    ev.type           = ctx->_NET_WM_STATE;
    ev.data.data32[0] = status ? 1 : 0; // _NET_WM_STATE_ADD / _NET_WM_STATE_REMOVE
    ev.data.data32[1] = ctx->_NET_WM_STATE_FULLSCREEN;
    ev.data.data32[2] = 0;
    ev.data.data32[3] = 1;
    ev.data.data32[4] = 0;

    xcb_send_event(c, 0, ctx->screen->root,
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
        (const char*) &ev);
    xcb_flush(c);

    if (!status)
    {
        // Restore saved position/size
        WND_NativeSavedData saved = WND_ToSavedData(window->savedData);
        u32 vals[4] = { (u32) saved.posX, (u32) saved.posY, saved.sizeX, saved.sizeY };
        xcb_configure_window(c, win,
            XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
            vals);
        xcb_flush(c);

        if (posX)  *posX  = saved.posX;
        if (posY)  *posY  = saved.posY;
        if (sizeX) *sizeX = saved.sizeX;
        if (sizeY) *sizeY = saved.sizeY;
    }
    else
    {
        // Save current dimensions before going fullscreen
        i16 cx, cy; u16 cw, ch;
        WND_GetDimensions(window, &cx, &cy, &cw, &ch);

        WND_NativeSavedData saved = { .posX = cx, .posY = cy, .sizeX = cw, .sizeY = ch };
        window->savedData = WND_FromSavedData(saved);

        if (posX)  *posX  = cx;
        if (posY)  *posY  = cy;
        if (sizeX) *sizeX = cw;
        if (sizeY) *sizeY = ch;
    }

    return true;
}

b8 WND_GetDimensions(WND_Data* window,
    i16* posX, i16* posY, u16* sizeX, u16* sizeY)
{
    if (!window) return false;
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return false;

    xcb_window_t win = (xcb_window_t) window->handle.handle;
    if (!win) return false;

    xcb_connection_t* c = ctx->connection;

    xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry(c, win);
    xcb_get_geometry_reply_t* geom       = xcb_get_geometry_reply(c, geomCookie, nil);
    if (!geom) return false;

    // translate origin to root coords
    xcb_translate_coordinates_cookie_t transCookie =
        xcb_translate_coordinates(c, win, ctx->screen->root, 0, 0);
    xcb_translate_coordinates_reply_t* trans =
        xcb_translate_coordinates_reply(c, transCookie, nil);

    i16 x = trans ? (i16) trans->dst_x : (i16) geom->x;
    i16 y = trans ? (i16) trans->dst_y : (i16) geom->y;
    u16 w = (u16) geom->width;
    u16 h = (u16) geom->height;

    free(geom);
    if (trans) free(trans);

    if (posX)  *posX  = x;
    if (posY)  *posY  = y;
    if (sizeX) *sizeX = w;
    if (sizeY) *sizeY = h;
    return true;
}

b8 WND_GetPtrPos(WND_Data* window, i16* posX, i16* posY)
{
    if (!window) return false;
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return false;

    xcb_window_t win = (xcb_window_t) window->handle.handle;
    if (!win) return false;

    xcb_query_pointer_cookie_t cookie = xcb_query_pointer(ctx->connection, win);
    xcb_query_pointer_reply_t* reply  = xcb_query_pointer_reply(ctx->connection, cookie, nil);
    if (!reply) return false;

    i16 x = (i16) reply->win_x;
    i16 y = (i16) reply->win_y;
    free(reply);

    if (posX) *posX = x;
    if (posY) *posY = y;
    return true;
}

b8 WND_Rename(WND_Data* window, utf8str newName)
{
    if (!window) return false;
    WND_XCBContext* ctx = WND_GetXCBContext();
    if (!ctx) return false;

    xcb_window_t win = (xcb_window_t) window->handle.handle;
    if (!win) return false;

    xcb_connection_t* c = ctx->connection;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, win,
        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
        (u32) newName.count, newName.data);
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, win,
        ctx->_NET_WM_NAME, ctx->UTF8_STRING, 8,
        (u32) newName.count, newName.data);
    xcb_flush(c);
    return true;
}

#endif

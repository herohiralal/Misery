#pragma once
#include <__init.h>

#if !MSR_DBG || !MSR_WINDOWS
    // raddbg is only supported on windows
    // and we obviously don't want it in release builds
    #define RADDBG_MARKUP_STUBS 1
#endif
#include "ExtDeps/raddbg/raddbg_markup.h"
#undef RADDBG_MARKUP_STUBS

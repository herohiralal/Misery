#pragma once
#include <Renderer_Mtl/Renderer_Mtl.h>

#if REN_MTL
static inline NSString* REN_MtlMakeNSString(utf8str str)
{
    cstring cStr = STR_CloneToCStr(str, MEM_temp);
    return [NSString stringWithUTF8String:cStr ? cStr : ""];
}
#endif

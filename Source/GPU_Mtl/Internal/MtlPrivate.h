#pragma once
#include <GPU_Mtl/GPU_Mtl.h>

#if GPU_MTL
static inline NSString* GPU_MtlMakeNSString(utf8str str)
{
    cstring cStr = STR_CloneToCStr(str, MEM_temp);
    return [NSString stringWithUTF8String:cStr ? cStr : ""];
}
#endif

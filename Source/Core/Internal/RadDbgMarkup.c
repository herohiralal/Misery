#include <Core/Core.h>

#if MSR_DBG && MSR_WINDOWS

MSR_SUPPRESS_WARN
#define RADDBG_MARKUP_IMPLEMENTATION 1
#include <Core/raddbg/raddbg_markup.h>
#undef RADDBG_MARKUP_IMPLEMENTATION
MSR_UNSUPPRESS_WARN

raddbg_type_view(b8, bool($));
raddbg_type_view(COL_Slice_?, array(data, count));
raddbg_type_view(COL_List_?, array(data, count));
raddbg_type_view(utf8str, array(data, count));

#endif

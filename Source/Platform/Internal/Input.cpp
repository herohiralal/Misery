#include "InputPrivate.h"

INP_Internal_State* INP_Internal_GetState(void)
{
    struct InputState final
    {
        MEM_ArenaAllocator a;
        INP_Internal_State s;

        InputState()
        {
            a = MEM_CreateArenaAllocator(1 * 1024 * 1024, MEM_main);

            s = INP_Internal_State { };
            s.tempAllocator = MEM_AllocatorFromArena(&a);

            s.droppedFiles = COL_NewList(utf8str, 0, s.tempAllocator);
            s.resizes = COL_NewList(INP_WindowResizeData, 0, s.tempAllocator);
            s.moves = COL_NewList(INP_WindowMoveData, 0, s.tempAllocator);
            s.evts = COL_NewList(INP_Evt, 0, s.tempAllocator);

            s.keyStates = COL_NewSlice(INP_CurrentKeyState, (isize) INP_KC_NUM, true, MEM_main);

            #if MSR_WINDOWS
            {
                s.rawInputBuffer = COL_NewList(u8, 0, MEM_main);
                s.keysDown = COL_NewList(i32, 32, MEM_main);
            }
            #endif
        }

        ~InputState()
        {
            #if MSR_WINDOWS
            {
                COL_DeleteList(&(s.keysDown));
                COL_DeleteList(&(s.rawInputBuffer));
            }
            #endif

            COL_DeleteSlice(&(s.keyStates), MEM_main);

            MEM_DestroyArenaAllocator(&a);
        }
    };

    static InputState s = InputState();
    return &s.s;
}

// this library is sometimes needed because some apps (like appid 410900) will check
// for its existence inside their memory space during runtime,
// otherwise they'll trigger custom nagging stuff

// notice how functions return a boolean result as uint32 not bool
// the return value is placed inside the entire 'eax' register, not just 'al'
// you have to disassem both 32-bit and 64-bit libraries to see this nonsense
// ex: 32-bit ValveIsScreenshotsHooked(): mov al, byte ptr ds:[0x7AF62DA1]
//     64-bit ValveIsScreenshotsHooked(): movzx eax, byte ptr ds:[0x00007FF8938F235D]

#ifndef __GAME_OVERLAY_RENDERER_LIB_H__
#define __GAME_OVERLAY_RENDERER_LIB_H__

#include "dll/common_includes.h"


S_API_EXPORT steam_bool S_CALLTYPE BOverlayNeedsPresent();

S_API_EXPORT steam_bool S_CALLTYPE IsOverlayEnabled();

S_API_EXPORT void S_CALLTYPE OverlayHookD3D3(void *param_1, void *param_2);

S_API_EXPORT void S_CALLTYPE SetNotificationInset(int32 param_1, int32 param_2);

S_API_EXPORT void S_CALLTYPE SetNotificationPosition(ENotificationPosition param_1);

S_API_EXPORT steam_bool S_CALLTYPE SteamOverlayIsUsingGamepad();

S_API_EXPORT steam_bool S_CALLTYPE SteamOverlayIsUsingKeyboard();

S_API_EXPORT void S_CALLTYPE ValveHookScreenshots(bool param_1);

S_API_EXPORT steam_bool S_CALLTYPE ValveIsScreenshotsHooked();

// only available for 32-bit lib on Windows
// the function takes 2 arguments, but they're expected to be at [esp] & [esp+4]
// notice how the top of the stack is not the return address, but param 1
// this function might be designed to be invoked via a jmp instruction,
// at the end it does this:
//   popad  // pop all 8 general purpose registers
//   popfd  // pop flags register
//   ret    // regular return
// notice how it eventually deallocates 8+1 registers (4 bytes * 9),
// meaning that (4 bytes * 2 input params) + (4 bytes * 9) are deallocated from the stack

// note: this is not marked S_API_EXPORT, because later we'll use a linker pragma
// to both export this function, and prevent __stdcall name mangling
#if defined(__WINDOWS_32__)
void __stdcall VirtualFreeWrapper(
    void *param_1, void *param_2,
    void *stack_cleanup_1, void *stack_cleanup_2, void *stack_cleanup_3, void *stack_cleanup_4, void *stack_cleanup_5, 
    void *stack_cleanup_6, void *stack_cleanup_7, void *stack_cleanup_8, void *stack_cleanup_9
);
#endif // __WINDOWS_32__

S_API_EXPORT void S_CALLTYPE VulkanSteamOverlayGetScaleFactors(float *param_1, float *param_2);

S_API_EXPORT void S_CALLTYPE VulkanSteamOverlayPresent(
    void *param_1, int32 param_2, int32 param_3, void *param_4, void * param_5,
    void *param_6, void *param_7, void *param_8, void *param_9, void *param_10
);


S_API_EXPORT void S_CALLTYPE VulkanSteamOverlayProcessCapturedFrame(
    bool param_1, int32 param_2, int32 param_3, int32 param_4, void *param_5, void *param_6, int32 param_7,
    int32 param_8, int32 param_9, int32 param_10, int32 param_11, int16 param_12, int16 param_13, int16 param_14
);


#endif // __GAME_OVERLAY_RENDERER_LIB_H__

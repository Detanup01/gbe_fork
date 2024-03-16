#include "game_overlay_renderer_lib.h"

static bool overlay_enabled = false;
static bool screenshots_hooked = false;

static float scale_factor_x = 0.0f;
static float scale_factor_y = 0.0f;

static int32 notification_inset_x = 0;
static int32 notification_inset_y = 0;

static ENotificationPosition notification_position = ENotificationPosition::k_EPositionInvalid;

S_API_EXPORT steam_bool S_CALLTYPE BOverlayNeedsPresent()
{
    return false;
}


S_API_EXPORT steam_bool S_CALLTYPE IsOverlayEnabled()
{
    return overlay_enabled;
}


S_API_EXPORT void S_CALLTYPE OverlayHookD3D3(void *param_1, void *param_2)
{

}

S_API_EXPORT void S_CALLTYPE SetNotificationInset(int32 param_1, int32 param_2)
{
    if (param_1 >= 0 && param_2 >=0) {
        notification_inset_x = param_1;
        notification_inset_y = param_2;
    }
}

S_API_EXPORT void S_CALLTYPE SetNotificationPosition(ENotificationPosition param_1)
{
    switch (param_1)
    {
    case ENotificationPosition::k_EPositionTopLeft:
    case ENotificationPosition::k_EPositionTopRight:
    case ENotificationPosition::k_EPositionBottomLeft:
    case ENotificationPosition::k_EPositionBottomRight:
        notification_position = param_1;
    break;
    
    default: break;
    }
}


S_API_EXPORT steam_bool S_CALLTYPE SteamOverlayIsUsingGamepad()
{
    return false;
}

S_API_EXPORT steam_bool S_CALLTYPE SteamOverlayIsUsingKeyboard()
{
    return false;
}

S_API_EXPORT void S_CALLTYPE ValveHookScreenshots(bool param_1)
{
    screenshots_hooked = param_1;
}

S_API_EXPORT steam_bool S_CALLTYPE ValveIsScreenshotsHooked()
{
    return screenshots_hooked;
}

#if defined(__WINDOWS_32__)
void __stdcall VirtualFreeWrapper(
    void *param_1, void *param_2,
    void *stack_cleanup_1, void *stack_cleanup_2, void *stack_cleanup_3, void *stack_cleanup_4, void *stack_cleanup_5, 
    void *stack_cleanup_6, void *stack_cleanup_7, void *stack_cleanup_8, void *stack_cleanup_9
)
{
    // https://stackoverflow.com/a/2805560
    #pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
}
#endif // __WINDOWS_32__

S_API_EXPORT void S_CALLTYPE VulkanSteamOverlayGetScaleFactors(float *param_1, float *param_2)
{
    // the original function doesn't check for nullptr
    if (param_1) *param_1 = 0.0f;
    if (param_2) *param_2 = 0.0f;
}

S_API_EXPORT void S_CALLTYPE VulkanSteamOverlayPresent(
    void *param_1, int32 param_2, int32 param_3, void *param_4, void * param_5,
    void *param_6, void *param_7, void *param_8, void *param_9, void *param_10
)
{
    
}


S_API_EXPORT void S_CALLTYPE VulkanSteamOverlayProcessCapturedFrame(
    bool param_1, int32 param_2, int32 param_3, int32 param_4, void *param_5, void *param_6, int32 param_7,
    int32 param_8, int32 param_9, int32 param_10, int32 param_11, int16 param_12, int16 param_13, int16 param_14
)
{
    
}

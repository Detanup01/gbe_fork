#pragma once

// common_includes.h pulls in the modern Steam SDK. Legacy steam_api_c exports use
// the same names as the modern SDK, so rename those symbols before including the
// SDK headers and then restore them afterwards. This keeps the legacy flat C ABI
// usable without conflicting with the current gbe_fork definitions.
#define SteamAPI_Init SteamAPI_Init_sdk
#define SteamAPI_Shutdown SteamAPI_Shutdown_sdk
#define SteamAPI_GetHSteamPipe SteamAPI_GetHSteamPipe_sdk
#define SteamAPI_WriteMiniDump SteamAPI_WriteMiniDump_sdk
#define SteamAPI_SetMiniDumpComment SteamAPI_SetMiniDumpComment_sdk
#define SteamAPI_UseBreakpadCrashHandler SteamAPI_UseBreakpadCrashHandler_sdk
#define SteamAPI_SetBreakpadAppID SteamAPI_SetBreakpadAppID_sdk
#define SteamFriends SteamFriends_sdk
#define SteamUser SteamUser_sdk
#define SteamGameServer_Init SteamGameServer_Init_sdk
#define SteamGameServer_Shutdown SteamGameServer_Shutdown_sdk
#define SteamGameServer_GetHSteamPipe SteamGameServer_GetHSteamPipe_sdk
#define SteamGameServer_RunCallbacks SteamGameServer_RunCallbacks_sdk

#include "dll/common_includes.h"

#undef SteamAPI_Init
#undef SteamAPI_Shutdown
#undef SteamAPI_GetHSteamPipe
#undef SteamAPI_WriteMiniDump
#undef SteamAPI_SetMiniDumpComment
#undef SteamAPI_UseBreakpadCrashHandler
#undef SteamAPI_SetBreakpadAppID
#undef SteamFriends
#undef SteamUser
#undef SteamGameServer_Init
#undef SteamGameServer_Shutdown
#undef SteamGameServer_GetHSteamPipe
#undef SteamGameServer_RunCallbacks
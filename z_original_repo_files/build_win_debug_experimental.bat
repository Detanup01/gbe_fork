@echo off
cd /d "%~dp0"
call build_set_protobuf_directories.bat
"%PROTOC_X86_EXE%" -I.\dll\ --cpp_out=.\dll\ .\dll\net.proto
call build_env_x86.bat
cl dll/rtlgenrandom.c dll/rtlgenrandom.def
cl /Z7 /LD /IImGui /I%PROTOBUF_X86_DIRECTORY%\include\ /DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /Ioverlay_experimental dll/*.cpp dll/*.cc detours/*.cpp controller/gamepad.c ImGui/*.cpp ImGui/backends/imgui_impl_dx*.cpp ImGui/backends/imgui_impl_win32.cpp ImGui/backends/imgui_impl_vulkan.cpp ImGui/backends/imgui_impl_opengl3.cpp ImGui/backends/imgui_win_shader_blobs.cpp overlay_experimental/*.cpp overlay_experimental/windows/*.cpp overlay_experimental/System/*.cpp "%PROTOBUF_X86_LIBRARY%" opengl32.lib Iphlpapi.lib Ws2_32.lib rtlgenrandom.lib Shell32.lib Winmm.lib /EHsc /MP12 /link /OUT:steam_api.dll
cl /LD steamclient.cpp /EHsc /MP12 /link /OUT:steamclient.dll

"%PROTOC_X64_EXE%" -I.\dll\ --cpp_out=.\dll\ .\dll\net.proto
call build_env_x64.bat
cl dll/rtlgenrandom.c dll/rtlgenrandom.def
cl /Z7 /LD /IImGui /I%PROTOBUF_X64_DIRECTORY%\include\ /DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /Ioverlay_experimental dll/*.cpp dll/*.cc detours/*.cpp controller/gamepad.c ImGui/*.cpp ImGui/backends/imgui_impl_dx*.cpp ImGui/backends/imgui_impl_win32.cpp ImGui/backends/imgui_impl_vulkan.cpp ImGui/backends/imgui_impl_opengl3.cpp ImGui/backends/imgui_win_shader_blobs.cpp overlay_experimental/*.cpp overlay_experimental/windows/*.cpp overlay_experimental/System/*.cpp "%PROTOBUF_X64_LIBRARY%" opengl32.lib Iphlpapi.lib Ws2_32.lib rtlgenrandom.lib Shell32.lib Winmm.lib /EHsc /MP12 /link /OUT:steam_api64.dll
cl /LD steamclient.cpp /EHsc /MP12 /link /OUT:steamclient64.dll

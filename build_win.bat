@echo off

setlocal
pushd "%~dp0"

set /a last_code=0

for %%A in (
  "dll\dll.cpp"
  "dll\steam_client.cpp"
  "controller\gamepad.c"
  "sdk\steam\isteamclient.h" ) do (
    if not exist "%%~A" (
      call :err_msg "Invalid emu directory, change directory to emu's src dir (missing file %%~A)"
      set /a last_code=1
      goto :end_script
    )
)

set /a BUILD_LIB32=1
set /a BUILD_LIB64=1

set /a BUILD_EXP_LIB32=1
set /a BUILD_EXP_LIB64=1
set /a BUILD_EXP_CLIENT32=1
set /a BUILD_EXP_CLIENT64=1

set /a BUILD_EXPCLIENT32=1
set /a BUILD_EXPCLIENT64=1
set /a BUILD_EXPCLIENT_LDR_32=1
set /a BUILD_EXPCLIENT_LDR_64=1

set /a BUILD_EXPCLIENT_EXTRA_32=0
set /a BUILD_EXPCLIENT_EXTRA_64=0

set /a BUILD_TOOL_FIND_ITFS=1
set /a BUILD_TOOL_LOBBY=1

set /a BUILD_LIB_NET_SOCKETS_32=0
set /a BUILD_LIB_NET_SOCKETS_64=0

:: < 0: deduce, > 1: force
set /a PARALLEL_THREADS_OVERRIDE=-1

:: 0 = release, 1 = debug, otherwise error
set /a BUILD_TYPE=-1

set /a CLEAN_BUILD=0

set /a VERBOSE=0

:: get args
:args_loop
  if "%~1"=="" (
    goto :args_loop_end
  ) else if "%~1"=="-lib-32" (
    set /a BUILD_LIB32=0
  ) else if "%~1"=="-lib-64" (
    set /a BUILD_LIB64=0
  ) else if "%~1"=="-ex-lib-32" (
    set /a BUILD_EXP_LIB32=0
  ) else if "%~1"=="-ex-lib-64" (
    set /a BUILD_EXP_LIB64=0
  ) else if "%~1"=="-ex-client-32" (
    set /a BUILD_EXP_CLIENT32=0
  ) else if "%~1"=="-ex-client-64" (
    set /a BUILD_EXP_CLIENT64=0
  ) else if "%~1"=="-exclient-32" (
    set /a BUILD_EXPCLIENT32=0
  ) else if "%~1"=="-exclient-64" (
    set /a BUILD_EXPCLIENT64=0
  ) else if "%~1"=="-exclient-ldr-32" (
    set /a BUILD_EXPCLIENT_LDR_32=0
  ) else if "%~1"=="-exclient-ldr-64" (
    set /a BUILD_EXPCLIENT_LDR_64=0
  ) else if "%~1"=="+exclient-extra-32" (
    set /a BUILD_EXPCLIENT_EXTRA_32=1
  ) else if "%~1"=="+exclient-extra-64" (
    set /a BUILD_EXPCLIENT_EXTRA_64=1
  ) else if "%~1"=="-tool-itf" (
    set /a BUILD_TOOL_FIND_ITFS=0
  ) else if "%~1"=="-tool-lobby" (
    set /a BUILD_TOOL_LOBBY=0
  ) else if "%~1"=="+lib-netsockets-32" (
    set /a BUILD_LIB_NET_SOCKETS_32=1
  ) else if "%~1"=="+lib-netsockets-64" (
    set /a BUILD_LIB_NET_SOCKETS_64=1
  ) else if "%~1"=="-j" (
    call :get_parallel_threads_count %~2 || (
      call :err_msg "Invalid arg after -j, expected a number"
      set /a last_code=1
      goto :end_script
    )
    shift /1
  ) else if "%~1"=="-verbose" (
    set /a VERBOSE=1
  ) else if "%~1"=="clean" (
    set /a CLEAN_BUILD=1
  ) else if "%~1"=="release" (
    set /a BUILD_TYPE=0
  ) else if "%~1"=="debug" (
    set /a BUILD_TYPE=1
  ) else (
    call :err_msg "Invalid arg: %~1"
    set /a last_code=1
    goto :end_script
  )

  shift /1
  goto :args_loop
:args_loop_end

:: use 70%
if defined NUMBER_OF_PROCESSORS (
  set /a build_threads=NUMBER_OF_PROCESSORS*70/100
) else (
  set /a build_threads=2
)
if %PARALLEL_THREADS_OVERRIDE% gtr 0 (
  set /a build_threads=PARALLEL_THREADS_OVERRIDE
)
if %build_threads% lss 2 (
  set /a build_threads=2
)

:: build type
:: NOTE: don't use /D_DEBUG
:: this will use the debug version of most of the C/C++ runtime
:: but the deps are all built in release mode, resuting in errors like this:
:: libprotobuf-lite.lib(common.obj) : error LNK2038: mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MTd_StaticDebug' in auth.obj
set "debug_info="
set "debug_info_format="
set "optimization_level="
set "dbg_defs="
set "build_folder="
if %BUILD_TYPE% equ 0 (
  set "debug_info=/DEBUG:NONE"
  set "debug_info_format="
  set "optimization_level=/Ox /Oi /Ob2 /Ot /O2 /Oy-"
  set "dbg_defs=/DEMU_RELEASE_BUILD /DNDEBUG"
  set "build_folder=release"
) else if %BUILD_TYPE% equ 1 (
  set "debug_info=/DEBUG:FULL"
  set "debug_info_format=/Z7"
  set "optimization_level=/Od"
  set "dbg_defs="
  set "build_folder=debug"
) else (
  call :err_msg "You must specify any of: [release debug]"
  set /a last_code=1
  goto :end_script
)

set "build_root_dir=build\win\%build_folder%"
set "steamclient_dir=%build_root_dir%\experimental_steamclient"
set "experimental_dir=%build_root_dir%\experimental"
set "tools_dir=%build_root_dir%\tools"
set "find_interfaces_dir=%tools_dir%\find_interfaces"
set "lobby_connect_dir=%tools_dir%\lobby_connect"

:: common stuff
set "deps_dir=build\deps\win"
set "libs_dir=libs"
set "tools_src_dir=tools"
set "build_temp_dir=build\tmp\win"
set "protoc_out_dir=dll\proto_gen\win"
set "win_resources_src_dir=resources\win"
set "win_resources_out_dir=%build_temp_dir%\rsrc"
set "third_party_build_win_dir=third-party\build\win"
set "signer_tool=%third_party_build_win_dir%\cert\sign_helper.bat"

set "dos_stub_exe_32=%win_resources_src_dir%\file_dos_stub\file_dos_stub_32.exe"
set "dos_stub_exe_64=%win_resources_src_dir%\file_dos_stub\file_dos_stub_64.exe"

set "protoc_exe_32=%deps_dir%\protobuf\install32\bin\protoc.exe"
set "protoc_exe_64=%deps_dir%\protobuf\install64\bin\protoc.exe"

set "common_compiler_args=/std:c++17 /MP%build_threads% /DYNAMICBASE /errorReport:none /nologo /utf-8 /EHsc /GF /GL- /GS"
set "common_compiler_args_32=%common_compiler_args%"
set "common_compiler_args_64=%common_compiler_args%"

:: "win" variables are used to build .dll and /SUBSYTEM:WINDOWS applications,
:: "exe" variables are used to build pure console applications
set "common_linker_args=/DYNAMICBASE /ERRORREPORT:NONE /NOLOGO /emittoolversioninfo:no"
set "common_win_linker_args_32=%common_linker_args%"
set "common_win_linker_args_64=%common_linker_args%"
set "common_exe_linker_args_32=%common_linker_args%"
set "common_exe_linker_args_64=%common_linker_args%"

:: third party dependencies (include folder + exact .lib file location)
set ssq_inc=/I"%deps_dir%\libssq\include"
set ssq_lib32="%deps_dir%\libssq\build32\Release\ssq.lib"
set ssq_lib64="%deps_dir%\libssq\build64\Release\ssq.lib"

set curl_inc32=/I"%deps_dir%\curl\install32\include"
set curl_inc64=/I"%deps_dir%\curl\install64\include"
set curl_lib32="%deps_dir%\curl\install32\lib\libcurl.lib"
set curl_lib64="%deps_dir%\curl\install64\lib\libcurl.lib"

set protob_inc32=/I"%deps_dir%\protobuf\install32\include"
set protob_inc64=/I"%deps_dir%\protobuf\install64\include"
set protob_lib32="%deps_dir%\protobuf\install32\lib\libprotobuf-lite.lib"
set protob_lib64="%deps_dir%\protobuf\install64\lib\libprotobuf-lite.lib"

set zlib_inc32=/I"%deps_dir%\zlib\install32\include"
set zlib_inc64=/I"%deps_dir%\zlib\install64\include"
set zlib_lib32="%deps_dir%\zlib\install32\lib\zlibstatic.lib"
set zlib_lib64="%deps_dir%\zlib\install64\lib\zlibstatic.lib"

set mbedtls_inc32=/I"%deps_dir%\mbedtls\install32\include"
set mbedtls_inc64=/I"%deps_dir%\mbedtls\install64\include"
set mbedtls_lib32="%deps_dir%\mbedtls\install32\lib\mbedcrypto.lib"
set mbedtls_lib64="%deps_dir%\mbedtls\install64\lib\mbedcrypto.lib"

:: directories to use for #include
set release_incs_both=%ssq_inc% /I"%libs_dir%" /I"%protoc_out_dir%" /I"%libs_dir%\utfcpp" /I"controller" /I"dll" /I"sdk" /I"overlay_experimental" /I"crash_printer" /I"helpers"
set release_incs32=%release_incs_both% %curl_inc32% %protob_inc32% %zlib_inc32% %mbedtls_inc32%
set release_incs64=%release_incs_both% %curl_inc64% %protob_inc64% %zlib_inc64% %mbedtls_inc64%

:: libraries to link with
:: copied from Visual Studio 2022
set "CoreLibraryDependencies=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib"
set "release_libs_both=%CoreLibraryDependencies% Ws2_32.lib Iphlpapi.lib Wldap32.lib Winmm.lib Bcrypt.lib Dbghelp.lib"
set release_libs32=%release_libs_both% %ssq_lib32% %curl_lib32% %protob_lib32% %zlib_lib32% %mbedtls_lib32%
set release_libs64=%release_libs_both% %ssq_lib64% %curl_lib64% %protob_lib64% %zlib_lib64% %mbedtls_lib64%

:: common source files used everywhere, just for convinience, you still have to provide a complete list later
set release_src="dll/*.cpp" "%protoc_out_dir%/*.cc" "crash_printer/win.cpp" "helpers/common_helpers.cpp"

:: additional #defines
set "common_defs=/DUTF_CPP_CPLUSPLUS=201703L /DCURL_STATICLIB /DUNICODE /D_UNICODE"
set "release_defs=%dbg_defs% %common_defs%"


if not exist "%deps_dir%\" (
  call :err_msg "Dependencies dir was not found"
  set /a last_code=1
  goto :end_script
)

if not exist "%signer_tool%" (
  call :err_msg "signing tool wasn't found"
  set /a last_code=1
  goto :end_script
)

if not exist "%protoc_exe_32%" (
  call :err_msg "protobuff compiler wasn't found - 32"
  set /a last_code=1
  goto :end_script
)
if not exist "%protoc_exe_64%" (
  call :err_msg "protobuff compiler wasn't found - 64"
  set /a last_code=1
  goto :end_script
)

if not exist "%dos_stub_exe_32%" (
  call :err_msg "dos stub program wasn't found - 32"
  set /a last_code=1
  goto :end_script
)
if not exist "%dos_stub_exe_64%" (
  call :err_msg "dos stub program wasn't found - 64"
  set /a last_code=1
  goto :end_script
)

echo [?] All build operations will use %build_threads% parallel jobs

if %CLEAN_BUILD% equ 1 (
  echo // cleaning previous build
  if exist "%build_root_dir%" (
    rmdir /s /q "%build_root_dir%"
  )
  echo: & echo:
)

:: x32 build
setlocal

echo // cleaning up to build 32
call :cleanup

call build_win_set_env.bat 32 || (
  endlocal
  call :err_msg "Couldn't find Visual Studio or build tools - 32"
  set /a last_code=1
  goto :end_script
)

echo // invoking protobuf compiler - 32
rmdir /s /q "%protoc_out_dir%" >nul 2>&1
mkdir "%protoc_out_dir%"
"%protoc_exe_32%" .\dll\net.proto -I.\dll\ --cpp_out="%protoc_out_dir%\\" || (
  endlocal
  call :err_msg "Protobuf compiler failed - 32"
  set /a last_code=1
  goto :end_script
)
echo: & echo:

echo // building resources 32
call :build_rsrc "%win_resources_src_dir%\api\32\resources.rc" "%win_resources_out_dir%\rsrc-api-32.res" || (
  endlocal
  call :err_msg "Resource compiler failed - 32"
  set /a last_code=1
  goto :end_script
)
call :build_rsrc "%win_resources_src_dir%\client\32\resources.rc" "%win_resources_out_dir%\rsrc-client-32.res" || (
  endlocal
  call :err_msg "Resource compiler failed - 32"
  set /a last_code=1
  goto :end_script
)
call :build_rsrc "%win_resources_src_dir%\launcher\32\resources.rc" "%win_resources_out_dir%\rsrc-launcher-32.res" || (
  endlocal
  call :err_msg "Resource compiler failed - 32"
  set /a last_code=1
  goto :end_script
)
echo: & echo:

if %BUILD_LIB32% equ 1 (
  call :compile_lib32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXP_LIB32% equ 1 (
  call :compile_experimental_lib32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXP_CLIENT32% equ 1 (
  call :compile_experimental_client32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXPCLIENT32% equ 1 (
  call :compile_experimentalclient_32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

:: steamclient_loader
if %BUILD_EXPCLIENT_LDR_32% equ 1 (
  call :compile_experimentalclient_ldr_32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXPCLIENT_EXTRA_32% equ 1 (
  call :compile_experimentalclient_extra_32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

:: tools (x32)
if %BUILD_TOOL_FIND_ITFS% equ 1 (
  call :compile_tool_itf || (
    set /a last_code+=1
  )
  echo: & echo:
)
if %BUILD_TOOL_LOBBY% equ 1 (
  call :compile_tool_lobby || (
    set /a last_code+=1
  )
  echo: & echo:
)

:: networking sockets lib (x32)
if %BUILD_LIB_NET_SOCKETS_32% equ 1 (
  call :compile_networking_sockets_lib_32 || (
    set /a last_code+=1
  )
  echo: & echo:
)

endlocal & set /a last_code=%last_code%


:: some times the next build will fail, a timeout solved it
timeout /nobreak /t 5


:: x64 build
setlocal

echo // cleaning up to build 64
call :cleanup

call build_win_set_env.bat 64 || (
  endlocal
  call :err_msg "Couldn't find Visual Studio or build tools - 64"
  set /a last_code=1
  goto :end_script
)

echo // invoking protobuf compiler - 64
rmdir /s /q "%protoc_out_dir%" >nul 2>&1
mkdir "%protoc_out_dir%"
"%protoc_exe_64%" .\dll\net.proto -I.\dll\ --cpp_out="%protoc_out_dir%\\" || (
  endlocal
  call :err_msg "Protobuf compiler failed - 64"
  set /a last_code=1
  goto :end_script
)
echo: & echo:

echo // building resources 64
call :build_rsrc "%win_resources_src_dir%\api\64\resources.rc" "%win_resources_out_dir%\rsrc-api-64.res" || (
  endlocal
  call :err_msg "Resource compiler failed - 64"
  set /a last_code=1
  goto :end_script
)
call :build_rsrc "%win_resources_src_dir%\client\64\resources.rc" "%win_resources_out_dir%\rsrc-client-64.res" || (
  endlocal
  call :err_msg "Resource compiler failed - 64"
  set /a last_code=1
  goto :end_script
)
call :build_rsrc "%win_resources_src_dir%\launcher\64\resources.rc" "%win_resources_out_dir%\rsrc-launcher-64.res" || (
  endlocal
  call :err_msg "Resource compiler failed - 64"
  set /a last_code=1
  goto :end_script
)
echo: & echo:

if %BUILD_LIB64% equ 1 (
  call :compile_lib64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXP_LIB64% equ 1 (
  call :compile_experimental_lib64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXP_CLIENT64% equ 1 (
  call :compile_experimental_client64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXPCLIENT64% equ 1 (
  call :compile_experimentalclient_64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

:: steamclient_loader
if %BUILD_EXPCLIENT_LDR_64% equ 1 (
  call :compile_experimentalclient_ldr_64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

if %BUILD_EXPCLIENT_EXTRA_64% equ 1 (
  call :compile_experimentalclient_extra_64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

:: networking sockets lib (x64)
if %BUILD_LIB_NET_SOCKETS_64% equ 1 (
  call :compile_networking_sockets_lib_64 || (
    set /a last_code+=1
  )
  echo: & echo:
)

endlocal & set /a last_code=%last_code%


:: cleanup
echo // cleaning up
call :cleanup
echo: & echo:


:: copy configs + examples
if %last_code% equ 0 (
  echo // copying readmes + files examples
  xcopy /y /s /e /r "post_build\steam_settings.EXAMPLE\" "%build_root_dir%\steam_settings.EXAMPLE\"
  xcopy /y /s /e /r "post_build\win\ColdClientLoader.EXAMPLE\" "%steamclient_dir%\dll_injection.EXAMPLE\"
  copy /y "%tools_src_dir%\steamclient_loader\win\ColdClientLoader.ini" "%steamclient_dir%\"
  copy /y "post_build\README.release.md" "%build_root_dir%\"
  copy /y "CHANGELOG.md" "%build_root_dir%\"
  if %BUILD_TYPE%==1 (
    copy /y "post_build\README.debug.md" "%build_root_dir%\"
  )
  if exist "%experimental_dir%" (
    copy /y "post_build\README.experimental.md" "%experimental_dir%\"
  )
  if exist "%steamclient_dir%" (
    copy /y "post_build\README.experimental_steamclient.md" "%steamclient_dir%\"
  )
  if exist "%find_interfaces_dir%" (
    copy /y "post_build\README.find_interfaces.md" "%find_interfaces_dir%\"
  )
  if exist "%lobby_connect_dir%" (
    copy /y "post_build\README.lobby_connect.md" "%lobby_connect_dir%\"
  )
) else (
  call :err_msg "Not copying readmes or files examples due to previous errors"
)
echo: & echo:


goto :end_script



:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: x32 + tools
:compile_lib32
  setlocal
  echo // building lib steam_api.dll - 32
  set src_files="%win_resources_out_dir%\rsrc-api-32.res" %release_src%
  call :build_for 1 0 "%build_root_dir%\x32\steam_api.dll" src_files
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%build_root_dir%\x32\steam_api.dll"
    call "%signer_tool%" "%build_root_dir%\x32\steam_api.dll"
  )
endlocal & exit /b %_exit%

:compile_experimental_lib32
  setlocal
  echo // building lib experimental steam_api.dll - 32
  set src_files="%win_resources_out_dir%\rsrc-api-32.res" %release_src% "%libs_dir%\detours\*.cpp" controller/gamepad.c "%libs_dir%\ImGui\*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_dx*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_win32.cpp" "%libs_dir%\ImGui\backends\imgui_impl_vulkan.cpp" "%libs_dir%\ImGui\backends\imgui_impl_opengl3.cpp" "%libs_dir%\ImGui\backends\imgui_win_shader_blobs.cpp" "overlay_experimental\*.cpp" "overlay_experimental\windows\*.cpp" "overlay_experimental\System\*.cpp"
  set extra_inc_dirs=/I"%libs_dir%\ImGui"
  call :build_for 1 0 "%experimental_dir%\x32\steam_api.dll" src_files extra_inc_dirs "/DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /DImTextureID=ImU64"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%experimental_dir%\x32\steam_api.dll"
    call "%signer_tool%" "%experimental_dir%\x32\steam_api.dll"
  )
endlocal & exit /b %_exit%

:compile_experimental_client32
  setlocal
  echo // building lib experimental steamclient.dll - 32
  set src_files="%win_resources_out_dir%\rsrc-client-32.res" "steamclient\steamclient.cpp"
  call :build_for 1 0 "%experimental_dir%\x32\steamclient.dll" src_files "" "/DEMU_EXPERIMENTAL_BUILD"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%experimental_dir%\x32\steamclient.dll"
    call "%signer_tool%" "%experimental_dir%\x32\steamclient.dll"
  )
endlocal & exit /b %_exit%

:compile_experimentalclient_32
  setlocal
  echo // building lib steamclient.dll - 32
  set src_files="%win_resources_out_dir%\rsrc-client-32.res" %release_src% "%libs_dir%\detours\*.cpp" controller/gamepad.c "%libs_dir%\ImGui\*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_dx*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_win32.cpp" "%libs_dir%\ImGui\backends\imgui_impl_vulkan.cpp" "%libs_dir%\ImGui\backends\imgui_impl_opengl3.cpp" "%libs_dir%\ImGui\backends\imgui_win_shader_blobs.cpp" "overlay_experimental\*.cpp" "overlay_experimental\windows\*.cpp" "overlay_experimental\System\*.cpp"
  set extra_inc_dirs=/I"%libs_dir%\ImGui"
  call :build_for 1 0 "%steamclient_dir%\steamclient.dll" src_files extra_inc_dirs "/DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /DImTextureID=ImU64 /DSTEAMCLIENT_DLL"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%steamclient_dir%\steamclient.dll"
    call "%signer_tool%" "%steamclient_dir%\steamclient.dll"
  )
endlocal & exit /b %_exit%

:compile_experimentalclient_ldr_32
  setlocal
  echo // building executable steamclient_loader_32.exe - 32
  set src_files="%win_resources_out_dir%\rsrc-launcher-32.res" "%tools_src_dir%\steamclient_loader\win\*.cpp" "helpers\pe_helpers.cpp" "helpers\common_helpers.cpp" "helpers\dbg_log.cpp"
  set extra_inc_dirs=/I"%tools_src_dir%\steamclient_loader\win\extra_protection" /I"pe_helpers"
  set "extra_libs=user32.lib"
  call :build_for 1 2 "%steamclient_dir%\steamclient_loader_32.exe" src_files extra_inc_dirs "" "%extra_libs%"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%steamclient_dir%\steamclient_loader_32.exe"
    call "%signer_tool%" "%steamclient_dir%\steamclient_loader_32.exe"
  )
endlocal & exit /b %_exit%

:compile_experimentalclient_extra_32
  setlocal
  echo // building library steamclient_extra.dll - 32
  set src_files="%win_resources_out_dir%\rsrc-client-32.res" "%tools_src_dir%\steamclient_loader\win\extra_protection\*.cpp" "helpers\pe_helpers.cpp" "helpers\common_helpers.cpp" "%libs_dir%\detours\*.cpp"
  set extra_inc_dirs=/I"%tools_src_dir%\steamclient_loader\win\extra_protection" /I"pe_helpers"
  call :build_for 1 0 "%steamclient_dir%\extra_dlls\steamclient_extra.dll" src_files extra_inc_dirs
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%steamclient_dir%\extra_dlls\steamclient_extra.dll"
    call "%signer_tool%" "%steamclient_dir%\extra_dlls\steamclient_extra.dll"
  )
endlocal & exit /b %_exit%

:compile_tool_itf
  setlocal
  echo // building tool generate_interfaces_file.exe - 32
  set src_files="%tools_src_dir%\generate_interfaces\generate_interfaces.cpp"
  call :build_for 1 1 "%find_interfaces_dir%\generate_interfaces_file.exe" src_files
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call "%signer_tool%" "%find_interfaces_dir%\generate_interfaces_file.exe"
  )
endlocal & exit /b %_exit%

:compile_tool_lobby
  setlocal
  echo // building tool lobby_connect.exe - 32
  set src_files="%win_resources_out_dir%\rsrc-launcher-32.res" "%tools_src_dir%\lobby_connect\lobby_connect.cpp" %release_src%
  call :build_for 1 1 "%lobby_connect_dir%\lobby_connect.exe" src_files "" "/DNO_DISK_WRITES /DLOBBY_CONNECT" "Comdlg32.lib"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%lobby_connect_dir%\lobby_connect.exe"
    call "%signer_tool%" "%lobby_connect_dir%\lobby_connect.exe"
  )
endlocal & exit /b %_exit%

:compile_networking_sockets_lib_32
  setlocal
  echo // building library steamnetworkingsockets.dll - 32
  set src_files="networking_sockets_lib\steamnetworkingsockets.cpp" 
  call :build_for 1 0 "%build_root_dir%\networking_sockets_lib\steamnetworkingsockets.dll" src_files
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 1 "%build_root_dir%\networking_sockets_lib\steamnetworkingsockets.dll"
    call "%signer_tool%" "%build_root_dir%\networking_sockets_lib\steamnetworkingsockets.dll"
  )
endlocal & exit /b %_exit%



:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: x64
:compile_lib64
  setlocal
  echo // building lib steam_api64.dll - 64
  set src_files="%win_resources_out_dir%\rsrc-api-64.res" %release_src%
  call :build_for 0 0 "%build_root_dir%\x64\steam_api64.dll" src_files
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%build_root_dir%\x64\steam_api64.dll"
    call "%signer_tool%" "%build_root_dir%\x64\steam_api64.dll"
  )
endlocal & exit /b %_exit%

:compile_experimental_lib64
  setlocal
  echo // building lib experimental steam_api64.dll - 64
  set src_files="%win_resources_out_dir%\rsrc-api-64.res" %release_src% "%libs_dir%\detours\*.cpp" controller/gamepad.c "%libs_dir%\ImGui\*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_dx*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_win32.cpp" "%libs_dir%\ImGui\backends\imgui_impl_vulkan.cpp" "%libs_dir%\ImGui\backends\imgui_impl_opengl3.cpp" "%libs_dir%\ImGui\backends\imgui_win_shader_blobs.cpp" "overlay_experimental\*.cpp" "overlay_experimental\windows\*.cpp" "overlay_experimental\System\*.cpp"
  set extra_inc_dirs=/I"%libs_dir%\ImGui"
  call :build_for 0 0 "%experimental_dir%\x64\steam_api64.dll" src_files extra_inc_dirs "/DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /DImTextureID=ImU64"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%experimental_dir%\x64\steam_api64.dll"
    call "%signer_tool%" "%experimental_dir%\x64\steam_api64.dll"
  )
endlocal & exit /b %_exit%

:compile_experimental_client64
  setlocal
  echo // building lib experimental steamclient64.dll - 64
  set src_files="%win_resources_out_dir%\rsrc-client-64.res" "steamclient\steamclient.cpp"
  call :build_for 0 0 "%experimental_dir%\x64\steamclient64.dll" src_files "" "/DEMU_EXPERIMENTAL_BUILD"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%experimental_dir%\x64\steamclient64.dll"
    call "%signer_tool%" "%experimental_dir%\x64\steamclient64.dll"
  )
endlocal & exit /b %_exit%

:compile_experimentalclient_64
  setlocal
  echo // building lib steamclient64.dll - 64
  set src_files="%win_resources_out_dir%\rsrc-client-64.res" %release_src% "%libs_dir%\detours\*.cpp" controller/gamepad.c "%libs_dir%\ImGui\*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_dx*.cpp" "%libs_dir%\ImGui\backends\imgui_impl_win32.cpp" "%libs_dir%\ImGui\backends\imgui_impl_vulkan.cpp" "%libs_dir%\ImGui\backends\imgui_impl_opengl3.cpp" "%libs_dir%\ImGui\backends\imgui_win_shader_blobs.cpp" "overlay_experimental\*.cpp" "overlay_experimental\windows\*.cpp" "overlay_experimental\System\*.cpp"
  set extra_inc_dirs=/I"%libs_dir%\ImGui"
  call :build_for 0 0 "%steamclient_dir%\steamclient64.dll" src_files extra_inc_dirs "/DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /DImTextureID=ImU64 /DSTEAMCLIENT_DLL"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%steamclient_dir%\steamclient64.dll"
    call "%signer_tool%" "%steamclient_dir%\steamclient64.dll"
  )
endlocal & exit /b %_exit%

:compile_experimentalclient_ldr_64
  setlocal
  echo // building executable steamclient_loader_64.exe - 64
  set src_files="%win_resources_out_dir%\rsrc-launcher-64.res" "%tools_src_dir%\steamclient_loader\win\*.cpp" "helpers\pe_helpers.cpp" "helpers\common_helpers.cpp" "helpers\dbg_log.cpp"
  set extra_inc_dirs=/I"%tools_src_dir%\steamclient_loader\win\extra_protection" /I"pe_helpers"
  set "extra_libs=user32.lib"
  call :build_for 0 2 "%steamclient_dir%\steamclient_loader_64.exe" src_files extra_inc_dirs "" "%extra_libs%"
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%steamclient_dir%\steamclient_loader_64.exe"
    call "%signer_tool%" "%steamclient_dir%\steamclient_loader_64.exe"
  )
endlocal & exit /b %_exit%

:compile_experimentalclient_extra_64
  setlocal
  echo // building library steamclient_extra64.dll - 64
  set src_files="%win_resources_out_dir%\rsrc-client-64.res" "%tools_src_dir%\steamclient_loader\win\extra_protection\*.cpp" "helpers\pe_helpers.cpp" "helpers\common_helpers.cpp" "%libs_dir%\detours\*.cpp"
  set extra_inc_dirs=/I"%tools_src_dir%\steamclient_loader\win\extra_protection" /I"pe_helpers"
  call :build_for 0 0 "%steamclient_dir%\extra_dlls\steamclient_extra64.dll" src_files extra_inc_dirs
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%steamclient_dir%\extra_dlls\steamclient_extra64.dll"
    call "%signer_tool%" "%steamclient_dir%\extra_dlls\steamclient_extra64.dll"
  )
endlocal & exit /b %_exit%

:compile_networking_sockets_lib_64
  setlocal
  echo // building library steamnetworkingsockets64.dll - 64
  set src_files="networking_sockets_lib\steamnetworkingsockets.cpp" 
  call :build_for 0 0 "%build_root_dir%\networking_sockets_lib\steamnetworkingsockets64.dll" src_files
  set /a _exit=%errorlevel%
  if %_exit% equ 0 (
    call :change_dos_stub 0 "%build_root_dir%\networking_sockets_lib\steamnetworkingsockets64.dll"
    call "%signer_tool%" "%build_root_dir%\networking_sockets_lib\steamnetworkingsockets64.dll"
  )
endlocal & exit /b %_exit%



:err_msg
  1>&2 echo [X] %~1
exit /b


:get_parallel_threads_count
  for /f "tokens=* delims=" %%A in ('echo %~1^| findstr /B /R /X "^[0-9][0-9]*$" 2^>nul') do (
    set /a PARALLEL_THREADS_OVERRIDE=%~1
    rem echo [?] Overriding parralel build jobs count with %~1
    exit /b 0
  )
exit /b 1


:: 1: is 32 bit build
:: 2: subsystem type [0: dll, 1: console app, 2: win32 app] 
::    check this table: https://learn.microsoft.com/en-us/cpp/build/reference/entry-entry-point-symbol#remarks
:: 3: out filepath
:: 4: (ref) all source files
:: 5: (optional) (ref) extra inc dirs
:: 6: (optional) extra defs
:: 7: (optional) extra libs
:build_for
  setlocal
  set /a _is_32_bit_build=%~1 2>nul || (
    endlocal
    call :err_msg "Missing build arch"
    exit /b 1
  )
  set /a _subsys_type=%~2 2>nul || (
    endlocal
    call :err_msg "Missing subsystem type"
    exit /b 1
  )
  set "_out_filepath=%~3"
  if "%_out_filepath%"=="" (
    endlocal
    call :err_msg "Missing output filepath"
    exit /b 1
  )
  set "_all_src="
  for /f "tokens=* delims=" %%A in (' if not "%~4" equ "" if defined %~4 echo %%%~4%%') do set _all_src=%%A
  if not defined _all_src (
    endlocal
    call :err_msg "Missing src files"
    exit /b 1
  )
  set "_extra_inc_dirs="
  for /f "tokens=* delims=" %%A in (' if not "%~5" equ "" if defined %~5 echo %%%~5%%') do set _extra_inc_dirs=%%A
  set "_extra_defs=%~6"
  set "_extra_libs=%~7"

  set "_build_tmp="
  for /f "usebackq tokens=* delims=" %%A in ('"%_out_filepath%"') do (
    set "_build_tmp=%build_temp_dir%\%%~nA_%_is_32_bit_build%"
  )
  if "%_build_tmp%"=="" (
    endlocal
    call :err_msg "Unable to set build tmp dir from filename"
    exit /b 1
  )
  mkdir "%_build_tmp%"

  set "_target_args=%common_compiler_args_64%"
  if %_is_32_bit_build% equ 1 (
    set "_target_args=%common_compiler_args_32%"
  )

  set _target_inc_dirs=%release_incs64%
  if %_is_32_bit_build% equ 1 (
    set _target_inc_dirs=%release_incs32%
  )

  set _target_libs=%release_libs64%
  if %_is_32_bit_build% equ 1 (
    set _target_libs=%release_libs32%
  )

  :: https://learn.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library
  :: https://learn.microsoft.com/en-us/cpp/build/reference/dll-build-a-dll
  set "_runtime_type=/MT"
  set "_target_linker_args="
  :: [0: dll, 1: console app, 2: win32 app]
  if %_subsys_type% equ 0 (
    set "_runtime_type=/MT /LD"
    if %_is_32_bit_build% equ 1 (
      set "_target_linker_args=%common_win_linker_args_32%"
    ) else (
      set "_target_linker_args=%common_win_linker_args_64%"
    )
  ) else if %_subsys_type% equ 1 (
    if %_is_32_bit_build% equ 1 (
      set "_target_linker_args=%common_exe_linker_args_32%"
    ) else (
      set "_target_linker_args=%common_exe_linker_args_64%"
    )
  ) else if %_subsys_type% equ 2 (
    if %_is_32_bit_build% equ 1 (
      set "_target_linker_args=%common_win_linker_args_32%"
    ) else (
      set "_target_linker_args=%common_win_linker_args_64%"
    )
  ) else (
    endlocal
    call :err_msg "Unknown subsystem type"
    exit /b 1
  )

  for /f "usebackq tokens=* delims=" %%A in ('"%_out_filepath%"') do (
    if not exist "%%~dpA" (
      mkdir "%%~dpA"
    )
  )

  if "%VERBOSE%" equ "1" (
    echo cl.exe %_target_args% /Fo%_build_tmp%\ /Fe%_build_tmp%\ %debug_info% %debug_info_format% %optimization_level% %release_defs% %_extra_defs% %_runtime_type% %_target_inc_dirs% %_extra_inc_dirs% %_all_src% %_target_libs% %_extra_libs% /link %_target_linker_args% /OUT:"%_out_filepath%"
    echo:
  )

  call cl.exe %_target_args% /Fo%_build_tmp%\ /Fe%_build_tmp%\ %debug_info% %debug_info_format% %optimization_level% %release_defs% %_extra_defs% %_runtime_type% %_target_inc_dirs% %_extra_inc_dirs% %_all_src% %_target_libs% %_extra_libs% /link %_target_linker_args% /OUT:"%_out_filepath%"
  set /a _exit=%errorlevel%
  rmdir /s /q "%_build_tmp%"
endlocal & exit /b %_exit%


:: 1: input filepath
:: 2: output filepath
:build_rsrc
  setlocal

  set "_file=%~1"
  if not exist "%_file%" (
    endlocal
    call :err_msg "Missing resource file"
    exit /b 1
  )

  set "_out_file=%~2"
  if not defined _out_file (
    endlocal
    call :err_msg "Missing output dir"
    exit /b 1
  )
  for /f "usebackq tokens=* delims=" %%A in ('"%_out_file%"') do (
    if not exist "%%~dpA" (
      mkdir "%%~dpA"
    )
  )
  
  echo --- compiling resource file "%_file%" to "%_out_file%"
  set "_verbose="
  if "%VERBOSE%" equ "1" (
    set "_verbose=/v"
  )
  if "%VERBOSE%" equ "1" (
    echo rc.exe %_verbose% /l 0 /g 0 /nologo /n /fo "%_out_file%" "%_file%"
    echo:
  )

  call rc.exe %_verbose% /l 0 /g 0 /nologo /n /fo "%_out_file%" "%_file%"
  set /a _exit=%errorlevel%
  echo:
endlocal & exit /b %_exit%


:: 1: is 32 bit build
:: 2: input filepath
:change_dos_stub
  setlocal
  set /a _is_32_bit_build=%~1 2>nul || (
    endlocal
    call :err_msg "Missing build arch"
    exit /b 1
  )
  set "_file=%~2"
  if not exist "%_file%" (
    endlocal
    call :err_msg "File not found"
    exit /b 1
  )

  if "%_is_32_bit_build%" equ "1" (
    set "_dos_stub_exe=%dos_stub_exe_32%"
  ) else (
    set "_dos_stub_exe=%dos_stub_exe_64%"
  )
  
  echo --- changing DOS stub of "%_file%"
  if "%VERBOSE%" equ "1" (
    echo "%_dos_stub_exe%" "%_file%"
    echo:
  )

  call "%_dos_stub_exe%" "%_file%"

endlocal & exit /b %errorlevel%


:cleanup
  del /f /q *.exp >nul 2>&1
  del /f /q *.lib >nul 2>&1
  del /f /q *.a >nul 2>&1
  del /f /q *.obj >nul 2>&1
  del /f /q *.pdb >nul 2>&1
  del /f /q *.ilk >nul 2>&1
  rmdir /s /q "%build_temp_dir%" >nul 2>&1
  rmdir /s /q "%win_resources_out_dir%" >nul 2>&1
  for %%A in ("ilk" "lib" "exp") do (
    del /f /s /q "%build_root_dir%\*.%%~A" >nul 2>&1
  )
exit /b


:end_script
echo:
if %last_code% equ 0 (
  echo [GG] no failures
) else (
  1>&2 echo [XX] general failure
)
popd
endlocal & (
  exit /b %last_code%
)

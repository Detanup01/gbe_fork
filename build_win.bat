@echo off

setlocal
pushd "%~dp0"

set /a last_code=0

set /a BUILD_LIB32=1
set /a BUILD_LIB64=1

set /a BUILD_EXP_LIB32=1
set /a BUILD_EXP_LIB64=1
set /a BUILD_EXP_CLIENT32=1
set /a BUILD_EXP_CLIENT64=1

set /a BUILD_EXPCLIENT32=1
set /a BUILD_EXPCLIENT64=1
set /a BUILD_EXPCLIENT_LDR=1

set /a BUILD_TOOL_FIND_ITFS=1
set /a BUILD_TOOL_LOBBY=1

set /a BUILD_WINXP=0
set /a BUILD_LOW_PERF=0

:: < 0: deduce, > 1: force
set /a PARALLEL_THREADS_OVERRIDE=-1

:: 0 = release, 1 = debug, otherwise error
set /a BUILD_TYPE=-1

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
  ) else if "%~1"=="-exclient-ldr" (
    set /a BUILD_EXPCLIENT_LDR=0
  ) else if "%~1"=="-tool-itf" (
    set /a BUILD_TOOL_FIND_ITFS=0
  ) else if "%~1"=="-tool-lobby" (
    set /a BUILD_TOOL_LOBBY=0
  ) else if "%~1"=="+lowperf" (
    set /a BUILD_LOW_PERF=1
  ) else if "%~1"=="-j" (
    call :get_parallel_threads_count %~2 || (
      call :err_msg "Invalid arg after -j, expected a number"
      set /a last_code=1
      goto :end_script
    )
    shift /1
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

set "additional_compiler_args_32="
set "additional_compiler_args_64="

set "additional_win_linker_args_32="
set "additional_win_linker_args_64="

set "additional_exe_linker_args_32="
set "additional_exe_linker_args_64="
:: win xp stuff
:: https://stackoverflow.com/a/29804473
:: https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
:: XXXX THIS DOESN"T WORK XXXX
:: XXXX emu code (networking) uses some win APIs available only starting from win Vista XXXX
if %BUILD_WINXP% equ 1 (
  set "additional_compiler_args_32=%additional_compiler_args_32% /D_USING_V110_SDK71_ /DNTDDI_VERSION=0x05010000 /D_WIN32_WINNT=0x0501 /DWINVER=0x0501"
  set "additional_compiler_args_64=%additional_compiler_args_64% /D_USING_V110_SDK71_ /DNTDDI_VERSION=0x05010000 /D_WIN32_WINNT=0x0501 /DWINVER=0x0501"

  set "additional_win_linker_args_32=%additional_win_linker_args_32% /SUBSYSTEM:WINDOWS,5.01"
  set "additional_win_linker_args_64=%additional_win_linker_args_64% /SUBSYSTEM:WINDOWS,5.02"

  set "additional_exe_linker_args_32=%additional_exe_linker_args_32% /SUBSYSTEM:CONSOLE,5.01"
  set "additional_exe_linker_args_64=%additional_exe_linker_args_64% /SUBSYSTEM:CONSOLE,5.02"
  
  set "build_folder=%build_folder%-win_xp"
)

if %BUILD_LOW_PERF% equ 1 (
  echo [?] Adding low perf flags
  set "additional_compiler_args_32=%additional_compiler_args_32% /arch:IA32"
  
  set "build_folder=%build_folder%-low_perf"
)


:: common stuff
set "deps_dir=build-win-deps"
set "build_root_dir=build-win\%build_folder%"
set "build_temp_dir=build-win-temp"
set "tools_dir=%build_root_dir%\tools"
set "steamclient_dir=%build_root_dir%\experimental_steamclient"
set "experimental_dir=%build_root_dir%\experimental"
set "find_interfaces_dir=%tools_dir%\find_interfaces"
set "lobby_connect_dir=%tools_dir%\lobby_connect"

set "common_compiler_args=/std:c++17 /MP%build_threads% /DYNAMICBASE /errorReport:none /nologo /utf-8 /EHsc /GF /GL- /GS /Fo%build_temp_dir%\ /Fe%build_temp_dir%\"
set "common_compiler_args_32=%common_compiler_args% %additional_compiler_args_32%"
set "common_compiler_args_64=%common_compiler_args% %additional_compiler_args_64%"

set "common_linker_args=/DYNAMICBASE /ERRORREPORT:NONE /NOLOGO"
set "common_win_linker_args_32=%common_linker_args% %additional_win_linker_args_32%"
set "common_win_linker_args_64=%common_linker_args% %additional_win_linker_args_64%"
set "common_exe_linker_args_32=%common_linker_args% %additional_exe_linker_args_32%"
set "common_exe_linker_args_64=%common_linker_args% %additional_exe_linker_args_64%"

set ssq_inc=/I"%deps_dir%\ssq\include"
set ssq_lib32="%deps_dir%\ssq\build32\Release\ssq.lib"
set ssq_lib64="%deps_dir%\ssq\build64\Release\ssq.lib"

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

set release_incs_both=%ssq_inc% /Iutfcpp /Ififo_map
set release_incs32=%release_incs_both% %curl_inc32% %protob_inc32% %zlib_inc32%
set release_incs64=%release_incs_both% %curl_inc64% %protob_inc64% %zlib_inc64%

set "common_defs=/DUTF_CPP_CPLUSPLUS=201703L /DCURL_STATICLIB /D_MT /DUNICODE /D_UNICODE"
set "release_defs=%dbg_defs% %common_defs%"

:: copied from Visual Studio 2022
set "CoreLibraryDependencies=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib"
set "release_libs_both=%CoreLibraryDependencies% Ws2_32.lib Iphlpapi.lib Wldap32.lib Winmm.lib Bcrypt.lib"

set release_libs32=%release_libs_both% %ssq_lib32% %curl_lib32% %protob_lib32% %zlib_lib32%
set release_libs64=%release_libs_both% %ssq_lib64% %curl_lib64% %protob_lib64% %zlib_lib64%

set "protoc_exe_32=%deps_dir%\protobuf\install32\bin\protoc.exe"
set "protoc_exe_64=%deps_dir%\protobuf\install64\bin\protoc.exe"

if not exist "%deps_dir%\" (
  call :err_msg "Dependencies dir was not found"
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

echo [?] All build operations will use %build_threads% parallel jobs

:: prepare folders
echo // preparing build folders
rmdir /s /q "%build_root_dir%\" >nul 2>&1
mkdir "%build_root_dir%\x32"
mkdir "%build_root_dir%\x64"

mkdir "%experimental_dir%\x32"
mkdir "%experimental_dir%\x64"

mkdir "%steamclient_dir%"

mkdir "%find_interfaces_dir%"
mkdir "%lobby_connect_dir%"


:: x32 build
setlocal

echo // cleaning up to build 32
call build_win_clean.bat
mkdir "%build_temp_dir%"

call build_win_set_env.bat 32 || (
  endlocal
  call :err_msg "Couldn't find Visual Studio or build tools - 32"
  set /a last_code=1
  goto :end_script
)

echo // invoking protobuf compiler - 32
"%protoc_exe_32%" -I.\dll\ --cpp_out=.\dll\ .\dll\net.proto || (
  set /a last_code+=1
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
if %BUILD_EXPCLIENT_LDR% equ 1 (
  call :compile_experimentalclient_ldr || (
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

endlocal & set /a last_code=%last_code%


:: some times the next build will fail, a timeout solved it
timeout /nobreak /t 5


:: x64 build
setlocal

echo // cleaning up to build 64
call build_win_clean.bat
mkdir "%build_temp_dir%"

call build_win_set_env.bat 64 || (
  endlocal
  call :err_msg "Couldn't find Visual Studio or build tools - 64"
  set /a last_code=1
  goto :end_script
)

echo // invoking protobuf compiler - 64
"%protoc_exe_64%" -I.\dll\ --cpp_out=.\dll\ .\dll\net.proto || (
  set /a last_code+=1
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

endlocal & set /a last_code=%last_code%


:: copy configs + examples
if %last_code% equ 0 (
  echo // copying readmes + files examples
  xcopy /y /s "files_example\*" "%build_root_dir%\"
  copy /y steamclient_loader\ColdClientLoader.ini "%steamclient_dir%\"
  copy /y Readme_release.txt "%build_root_dir%\Readme.txt"
  copy /y Readme_experimental.txt "%experimental_dir%\Readme.txt"
  copy /y Readme_experimental_steamclient.txt "%steamclient_dir%\Readme.txt"
  copy /y Readme_generate_interfaces.txt "%find_interfaces_dir%\Readme.txt"
  copy /y Readme_lobby_connect.txt "%lobby_connect_dir%\Readme.txt"
) else (
  call :err_msg "Not copying readmes or files examples due to previous errors"
)
echo: & echo:


:: cleanup
echo // cleaning up
call build_win_clean.bat
for %%A in ("ilk" "lib" "exp") do (
  del /f /s /q "%build_root_dir%\*.%%~A" >nul 2>&1
)
echo: & echo:


goto :end_script



:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: x32 + tools
:compile_lib32
  echo // building lib steam_api.dll - 32
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% /LD %release_incs32% dll/*.cpp dll/*.cc %release_libs32% /link %common_win_linker_args_32% /DLL /OUT:"%build_root_dir%\x32\steam_api.dll"
exit /b %errorlevel%

:compile_experimental_lib32
  echo // building lib experimental steam_api.dll - 32
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /LD %release_incs32% /IImGui /Ioverlay_experimental dll/*.cpp dll/*.cc detours/*.cpp controller/gamepad.c ImGui/*.cpp ImGui/backends/imgui_impl_dx*.cpp ImGui/backends/imgui_impl_win32.cpp ImGui/backends/imgui_impl_vulkan.cpp ImGui/backends/imgui_impl_opengl3.cpp ImGui/backends/imgui_win_shader_blobs.cpp overlay_experimental/*.cpp overlay_experimental/windows/*.cpp overlay_experimental/System/*.cpp %release_libs32% /link %common_win_linker_args_32% /DLL /OUT:"%experimental_dir%\x32\steam_api.dll"
exit /b %errorlevel%

:compile_experimental_client32
  echo // building lib experimental steamclient.dll - 32
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DEMU_EXPERIMENTAL_BUILD /LD %release_incs32% steamclient.cpp %release_libs32% /link %common_win_linker_args_32% /DLL /OUT:"%experimental_dir%\x32\steamclient.dll"
exit /b %errorlevel%

:compile_experimentalclient_32
  echo // building lib steamclient.dll - 32
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /DSTEAMCLIENT_DLL /LD %release_incs32% /IImGui /Ioverlay_experimental dll/*.cpp dll/*.cc detours/*.cpp controller/gamepad.c ImGui/*.cpp ImGui/backends/imgui_impl_dx*.cpp ImGui/backends/imgui_impl_win32.cpp ImGui/backends/imgui_impl_vulkan.cpp ImGui/backends/imgui_impl_opengl3.cpp ImGui/backends/imgui_win_shader_blobs.cpp overlay_experimental/*.cpp overlay_experimental/windows/*.cpp overlay_experimental/System/*.cpp %release_libs32% /link %common_win_linker_args_32% /DLL /OUT:"%steamclient_dir%\steamclient.dll"
exit /b %errorlevel%

:compile_experimentalclient_ldr
  echo // building executable steamclient_loader.exe - 32
  :: common_win_linker_args_32 isn't a mistake, the entry is wWinMain
  :: check this table: https://learn.microsoft.com/en-us/cpp/build/reference/entry-entry-point-symbol#remarks
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% %release_incs32% steamclient_loader/*.cpp %release_libs32% user32.lib /link %common_win_linker_args_32% /OUT:"%steamclient_dir%\steamclient_loader.exe"
exit /b %errorlevel%

:compile_tool_itf
  echo // building tool generate_interfaces_file.exe - 32
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% %release_incs32% generate_interfaces_file.cpp %release_libs32% /link %common_exe_linker_args_32% /OUT:"%find_interfaces_dir%\generate_interfaces_file.exe"
exit /b %errorlevel%

:compile_tool_lobby
  echo // building tool lobby_connect.exe - 32
  cl %common_compiler_args_32% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DNO_DISK_WRITES /DLOBBY_CONNECT %release_incs32% lobby_connect.cpp dll/*.cpp dll/*.cc %release_libs32% Comdlg32.lib /link %common_exe_linker_args_32% /OUT:"%lobby_connect_dir%\lobby_connect.exe"
exit /b %errorlevel%



:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: x64
:compile_lib64
  echo // building lib steam_api64.dll - 64
  cl %common_compiler_args_64% %debug_info% %debug_info_format% %optimization_level% %release_defs% /LD %release_incs64% dll/*.cpp dll/*.cc %release_libs64% /link %common_win_linker_args_64% /DLL /OUT:"%build_root_dir%\x64\steam_api64.dll"
exit /b %errorlevel%

:compile_experimental_lib64
  echo // building lib experimental steam_api64.dll - 64
  cl %common_compiler_args_64% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /LD %release_incs64% /IImGui /Ioverlay_experimental dll/*.cpp dll/*.cc detours/*.cpp controller/gamepad.c ImGui/*.cpp ImGui/backends/imgui_impl_dx*.cpp ImGui/backends/imgui_impl_win32.cpp ImGui/backends/imgui_impl_vulkan.cpp ImGui/backends/imgui_impl_opengl3.cpp ImGui/backends/imgui_win_shader_blobs.cpp overlay_experimental/*.cpp overlay_experimental/windows/*.cpp overlay_experimental/System/*.cpp %release_libs64% opengl32.lib /link %common_win_linker_args_64% /DLL /OUT:"%experimental_dir%\x64\steam_api64.dll"
exit /b %errorlevel%

:compile_experimental_client64
  echo // building lib experimental steamclient64.dll - 64
  cl %common_compiler_args_64% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DEMU_EXPERIMENTAL_BUILD /LD %release_incs64% steamclient.cpp %release_libs64% /link %common_win_linker_args_64% /DLL /OUT:"%experimental_dir%\x64\steamclient64.dll"
exit /b %errorlevel%

:compile_experimentalclient_64
  echo // building lib steamclient64.dll - 64
  cl %common_compiler_args_64% %debug_info% %debug_info_format% %optimization_level% %release_defs% /DEMU_EXPERIMENTAL_BUILD /DCONTROLLER_SUPPORT /DEMU_OVERLAY /DSTEAMCLIENT_DLL /LD %release_incs64% /IImGui /Ioverlay_experimental dll/*.cpp dll/*.cc detours/*.cpp controller/gamepad.c ImGui/*.cpp ImGui/backends/imgui_impl_dx*.cpp ImGui/backends/imgui_impl_win32.cpp ImGui/backends/imgui_impl_vulkan.cpp ImGui/backends/imgui_impl_opengl3.cpp ImGui/backends/imgui_win_shader_blobs.cpp overlay_experimental/*.cpp overlay_experimental/windows/*.cpp overlay_experimental/System/*.cpp %release_libs64% opengl32.lib /link %common_win_linker_args_64% /DLL /OUT:"%steamclient_dir%\steamclient64.dll"
exit /b %errorlevel%



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

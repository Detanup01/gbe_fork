@echo off

setlocal
pushd "%~dp0"

set /a last_code=0

set /a VERBOSE=1
set "debug_info=/DEBUG:NONE"
set "debug_info_format="
set "optimization_level=/Ox /Oi /Ob2 /Ot /O2 /Oy-"
set "dbg_defs=/DEMU_RELEASE_BUILD /DNDEBUG"

set "build_root_dir=."

:: common stuff
set "build_temp_dir=build"

set "common_compiler_args=/std:c++17 /MP /DYNAMICBASE /errorReport:none /nologo /utf-8 /EHsc /GF /GL- /GS"
set "common_compiler_args_32=%common_compiler_args%"
set "common_compiler_args_64=%common_compiler_args%"

:: "win" variables are used to build .dll and /SUBSYTEM:WINDOWS applications,
:: "exe" variables are used to build pure console applications
set "common_linker_args=/DYNAMICBASE /ERRORREPORT:NONE /NOLOGO /emittoolversioninfo:no"
set "common_win_linker_args_32=%common_linker_args%"
set "common_win_linker_args_64=%common_linker_args%"
set "common_exe_linker_args_32=%common_linker_args%"
set "common_exe_linker_args_64=%common_linker_args%"

:: directories to use for #include
set release_incs_both=%ssq_inc% /I"../../../helpers"
set release_incs32=%release_incs_both%
set release_incs64=%release_incs_both%

:: libraries to link with
:: copied from Visual Studio 2022
set "CoreLibraryDependencies=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib"
set "release_libs_both=%CoreLibraryDependencies% Ws2_32.lib Iphlpapi.lib Wldap32.lib Winmm.lib Bcrypt.lib Dbghelp.lib"
set release_libs32=%release_libs_both%
set release_libs64=%release_libs_both%

:: common source files used everywhere, just for convinience, you still have to provide a complete list later
set release_src="file_dos_stub.cpp" "../../../helpers/pe_helpers.cpp" "../../../helpers/common_helpers.cpp"


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: x32
setlocal
call "../../../build_win_set_env.bat" 32 || (
  endlocal
  call :err_msg "Couldn't find Visual Studio or build tools - 32"
  set /a last_code=1
  goto :end_script
)

echo // building exe - 32
call :build_for 1 1 "%build_root_dir%\file_dos_stub_32.exe" release_src
set /a last_code+=%errorlevel%
endlocal & set /a last_code=%last_code%


timeout /t 3 /nobreak


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: x64
setlocal
call "../../../build_win_set_env.bat" 64 || (
  endlocal
  call :err_msg "Couldn't find Visual Studio or build tools - 64"
  set /a last_code=1
  goto :end_script
)

echo // building exe - 64
call :build_for 1 1 "%build_root_dir%\file_dos_stub_64.exe" release_src
set /a last_code+=%errorlevel%
endlocal & set /a last_code=%last_code%


:: cleanup
echo // cleaning up
call :cleanup
echo: & echo:


goto :end_script


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

  if "%VERBOSE%" equ "1" (
    echo cl.exe %_target_args% /Fo%_build_tmp%\ /Fe%_build_tmp%\ %debug_info% %debug_info_format% %optimization_level% %release_defs% %_extra_defs% %_runtime_type% %_target_inc_dirs% %_extra_inc_dirs% %_all_src% %_target_libs% %_extra_libs% /link %_target_linker_args% /OUT:"%_out_filepath%"
    echo:
  )

  cl.exe %_target_args% /Fo%_build_tmp%\ /Fe%_build_tmp%\ %debug_info% %debug_info_format% %optimization_level% %release_defs% %_extra_defs% %_runtime_type% %_target_inc_dirs% %_extra_inc_dirs% %_all_src% %_target_libs% %_extra_libs% /link %_target_linker_args% /OUT:"%_out_filepath%"
  set /a _exit=%errorlevel%
  rmdir /s /q "%_build_tmp%"
endlocal & exit /b %_exit%


:err_msg
  1>&2 echo [X] %~1
exit /b


:cleanup
  del /f /q *.exp >nul 2>&1
  del /f /q *.lib >nul 2>&1
  del /f /q *.a >nul 2>&1
  del /f /q *.obj >nul 2>&1
  del /f /q *.pdb >nul 2>&1
  del /f /q *.ilk >nul 2>&1
  rmdir /s /q "%build_temp_dir%" >nul 2>&1
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

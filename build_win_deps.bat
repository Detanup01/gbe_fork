@echo off

setlocal
pushd "%~dp0"

set "deps_dir=build\deps\win"
set "third_party_dir=third-party"
set "third_party_deps_dir=%third_party_dir%\deps\win"
set "third_party_common_dir=%third_party_dir%\deps\common"
set "extractor=%third_party_deps_dir%\7za\7za.exe"
set "mycmake=%~dp0%third_party_deps_dir%\cmake\bin\cmake.exe"

set /a last_code=0

if not exist "%extractor%" (
    call :err_msg "Couldn't find extraction tool"
    set /a last_code=1
    goto :end_script
)

if not exist "%mycmake%" (
    call :err_msg "Couldn't find cmake"
    set /a last_code=1
    goto :end_script
)

:: < 0: deduce, > 1: force
set /a PARALLEL_THREADS_OVERRIDE=-1

set "VERBOSITY="

:: get args
:args_loop
  if "%~1"=="" (
    goto :args_loop_end
  ) else if "%~1"=="-j" (
    call :get_parallel_threads_count %~2 || (
        call :err_msg "Invalid arg after -j, expected a number"
        set /a last_code=1
        goto :end_script
    )
    shift /1
  ) else if "%~1"=="-verbose" (
    set "VERBOSITY=-v"
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
    set /a jobs_count=NUMBER_OF_PROCESSORS*70/100
) else (
    set /a jobs_count=2
)

if %PARALLEL_THREADS_OVERRIDE% gtr 0 (
    set /a jobs_count=PARALLEL_THREADS_OVERRIDE
)

if %jobs_count% lss 2 (
    set /a jobs_count=2
)

call :extract_all_deps || (
    set /a last_code=1
    goto :end_script
)


:: ############## common CMAKE args ##############
:: https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_FLAGS_CONFIG.html#variable:CMAKE_%3CLANG%3E_FLAGS_%3CCONFIG%3E
set cmake_common_args=-G "Visual Studio 17 2022" -S .
set cmake_common_defs=-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_STANDARD_REQUIRED=ON -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_C_STANDARD=17 -DCMAKE_CXX_STANDARD=17 -DCMAKE_POSITION_INDEPENDENT_CODE=True -DBUILD_SHARED_LIBS=OFF -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded "-DCMAKE_C_FLAGS_RELEASE=/MT /std:c17 /D_MT" "-DCMAKE_CXX_FLAGS_RELEASE=/MT /std:c++17 /D_MT"
set "recreate_32=rmdir /s /q build32\ 1>nul 2>&1 & rmdir /s /q install32\ 1>nul 2>&1 & mkdir build32\ && mkdir install32\"
set "recreate_64=rmdir /s /q build64\ 1>nul 2>&1 & rmdir /s /q install64\ 1>nul 2>&1 & mkdir build64\ && mkdir install64\"
set cmake_gen32="%mycmake%" %cmake_common_args% -A Win32 -B build32 -DCMAKE_INSTALL_PREFIX=install32 %cmake_common_defs%
set cmake_gen64="%mycmake%" %cmake_common_args% -A x64 -B build64 -DCMAKE_INSTALL_PREFIX=install64 %cmake_common_defs%
set cmake_build32="%mycmake%" --build build32 --config Release --parallel %jobs_count% %VERBOSITY%
set cmake_build64="%mycmake%" --build build64 --config Release --parallel %jobs_count% %VERBOSITY%
set "clean_gen32=if exist build32\ rmdir /s /q build32"
set "clean_gen64=if exist build64\ rmdir /s /q build64"

:: "-DCMAKE_C_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib" "-DCMAKE_CXX_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib"

:: -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded" "-DMSVC_RUNTIME_LIBRARY=MultiThreaded" 

:: "-DCMAKE_C_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib"
:: "-DCMAKE_C_FLAGS_RELEASE=/MT /O2 /Ob2 /DNDEBUG"

:: "-DCMAKE_CXX_FLAGS_RELEASE=/MT /O2 /Ob2 /DNDEBUG"
:: "-DCMAKE_CXX_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib"

:: "-DCMAKE_C_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib" "-DCMAKE_C_FLAGS_RELEASE=/MT /O2 /Ob2 /DNDEBUG" 
 
::  "-DCMAKE_CXX_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib" 
:: "-DCMAKE_CXX_FLAGS_RELEASE=/MT /O2 /Ob2 /DNDEBUG" 

:: "-DCMAKE_C_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib" "-DCMAKE_CXX_STANDARD_LIBRARIES=kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib"

echo // [?] All CMAKE builds will use %jobs_count% parallel jobs

:: ############## build ssq ##############
echo // building ssq lib
pushd "%deps_dir%\libssq"

setlocal
call "%~dp0build_win_set_env.bat" 32 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 32"
    set /a last_code=1
    goto :end_script
)
%recreate_32%
%cmake_gen32%
set /a _exit=%errorlevel%
%cmake_build32%
set /a _exit+=%errorlevel%
endlocal & set /a last_code+=%_exit%

setlocal
call "%~dp0build_win_set_env.bat" 64 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 64"
    set /a last_code=1
    goto :end_script
)
%recreate_64%
%cmake_gen64%
set /a _exit=%errorlevel%
%cmake_build64%
set /a _exit+=%errorlevel%
endlocal & set /a last_code+=%_exit%

popd
echo: & echo:


:: ############## build zlib ##############
echo // building zlib lib
pushd "%deps_dir%\zlib"

setlocal
call "%~dp0build_win_set_env.bat" 32 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 32"
    set /a last_code=1
    goto :end_script
)
%recreate_32%
%cmake_gen32%
set /a _exit=%errorlevel%
%cmake_build32% --target install
set /a _exit+=%errorlevel%
%clean_gen32%
endlocal & set /a last_code+=%_exit%

setlocal
call "%~dp0build_win_set_env.bat" 64 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 64"
    set /a last_code=1
    goto :end_script
)
%recreate_64%
%cmake_gen64%
set /a _exit=%errorlevel%
%cmake_build64% --target install
set /a _exit+=%errorlevel%
%clean_gen64%
endlocal & set /a last_code+=%_exit%

popd
echo: & echo:


:: ############## zlib is painful ##############
:: lib curl uses the default search paths, even when ZLIB_INCLUDE_DIR and ZLIB_LIBRARY_RELEASE are defined
:: check thir CMakeLists.txt line #573
::     optional_dependency(ZLIB)
::     if(ZLIB_FOUND)
::       set(HAVE_LIBZ ON)
::       set(USE_ZLIB ON)
::     
::       # Depend on ZLIB via imported targets if supported by the running
::       # version of CMake.  This allows our dependents to get our dependencies
::       # transitively.
::       if(NOT CMAKE_VERSION VERSION_LESS 3.4)
::         list(APPEND CURL_LIBS ZLIB::ZLIB)    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< evil
::       else()
::         list(APPEND CURL_LIBS ${ZLIB_LIBRARIES})
::         include_directories(${ZLIB_INCLUDE_DIRS})
::       endif()
::       list(APPEND CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIRS})
::     endif()
:: we have to set the ZLIB_ROOT so that it is prepended to the search list
:: we have to set ZLIB_LIBRARY NOT ZLIB_LIBRARY_RELEASE in order to override the FindZlib module
:: we also should set ZLIB_USE_STATIC_LIBS since we want to force static builds
:: https://github.com/Kitware/CMake/blob/a6853135f569f0b040a34374a15a8361bb73901b/Modules/FindZLIB.cmake#L98C4-L98C13
set wild_zlib_32=-DZLIB_USE_STATIC_LIBS=ON "-DZLIB_ROOT=%~dp0%deps_dir%\zlib\install32" "-DZLIB_INCLUDE_DIR=%~dp0%deps_dir%\zlib\install32\include" "-DZLIB_LIBRARY=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib"
set wild_zlib_64=-DZLIB_USE_STATIC_LIBS=ON "-DZLIB_ROOT=%~dp0%deps_dir%\zlib\install64" "-DZLIB_INCLUDE_DIR=%~dp0%deps_dir%\zlib\install64\include" "-DZLIB_LIBRARY=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib"


:: ############## build curl ##############
echo // building curl lib
pushd "%deps_dir%\curl"

:: CURL_STATICLIB
set "curl_common_defs=-DBUILD_CURL_EXE=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_CURL=OFF -DBUILD_STATIC_LIBS=ON -DCURL_USE_OPENSSL=OFF -DCURL_ZLIB=ON -DENABLE_UNICODE=ON -DCURL_STATIC_CRT=ON"

setlocal
call "%~dp0build_win_set_env.bat" 32 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 32"
    set /a last_code=1
    goto :end_script
)
%recreate_32%
%cmake_gen32% %curl_common_defs% %wild_zlib_32% "-DCMAKE_SHARED_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib" "-DCMAKE_MODULE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib" "-DCMAKE_EXE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib"
set /a _exit=%errorlevel%
%cmake_build32% --target install
set /a _exit+=%errorlevel%
%clean_gen32%
endlocal & set /a last_code+=%_exit%

setlocal
call "%~dp0build_win_set_env.bat" 64 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 64"
    set /a last_code=1
    goto :end_script
)
%recreate_64%
%cmake_gen64% %curl_common_defs% %wild_zlib_64% "-DCMAKE_SHARED_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib" "-DCMAKE_MODULE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib" "-DCMAKE_EXE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib"
set /a _exit=%errorlevel%
%cmake_build64% --target install
set /a _exit+=%errorlevel%
%clean_gen64%
endlocal & set /a last_code+=%_exit%

popd
echo: & echo:


:: ############## build protobuf ##############
echo // building protobuf lib
pushd "%deps_dir%\protobuf"

set "proto_common_defs=-Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_WITH_ZLIB=ON"

setlocal
call "%~dp0build_win_set_env.bat" 32 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 32"
    set /a last_code=1
    goto :end_script
)
%recreate_32%
%cmake_gen32% %proto_common_defs% %wild_zlib_32% "-DCMAKE_SHARED_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib" "-DCMAKE_MODULE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib" "-DCMAKE_EXE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install32\lib\zlibstatic.lib"
set /a _exit=%errorlevel%
%cmake_build32% --target install
set /a _exit+=%errorlevel%
%clean_gen32%
endlocal & set /a last_code+=%_exit%

setlocal
call "%~dp0build_win_set_env.bat" 64 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 64"
    set /a last_code=1
    goto :end_script
)
%recreate_64%
%cmake_gen64% %proto_common_defs% %wild_zlib_64% "-DCMAKE_SHARED_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib" "-DCMAKE_MODULE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib" "-DCMAKE_EXE_LINKER_FLAGS_INIT=%~dp0%deps_dir%\zlib\install64\lib\zlibstatic.lib"
set /a _exit=%errorlevel%
%cmake_build64% --target install
set /a _exit+=%errorlevel%
%clean_gen64%
endlocal & set /a last_code+=%_exit%

popd
echo: & echo:


:: ############## build mbedtls ##############
echo // building mbedtls lib
pushd "%deps_dir%\mbedtls"

set "mbedtls_common_defs=-DUSE_STATIC_MBEDTLS_LIBRARY=ON -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DMSVC_STATIC_RUNTIME=ON -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF"

setlocal
call "%~dp0build_win_set_env.bat" 32 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 32"
    set /a last_code=1
    goto :end_script
)
%recreate_32%
%cmake_gen32% %mbedtls_common_defs%
set /a _exit=%errorlevel%
%cmake_build32% --target install
set /a _exit+=%errorlevel%
%clean_gen32%
endlocal & set /a last_code+=%_exit%

setlocal
call "%~dp0build_win_set_env.bat" 64 || (
    endlocal
    popd
    call :err_msg "Couldn't find Visual Studio or build tools - 64"
    set /a last_code=1
    goto :end_script
)
%recreate_64%
%cmake_gen64% %mbedtls_common_defs%
set /a _exit=%errorlevel%
%cmake_build64% --target install
set /a _exit+=%errorlevel%
%clean_gen64%
endlocal & set /a last_code+=%_exit%

popd
echo: & echo:


goto :end_script


:extract_all_deps
    set /a list=-1
    for /f "tokens=1 delims=:" %%A in ('findstr /B /L /N /C:"deps_to_extract=[" "%~f0" 2^>nul') do (
        set /a "list=%%~A"
    )
    if "%list%"=="-1" (
        call :err_msg "Couldn't find list of tools to extract inside thif batch script"
        exit /b 1
    )
    
    echo // Recreating dir...
    rmdir /s /q "%deps_dir%"
    mkdir "%deps_dir%"
    for /f "usebackq eol=; skip=%list% tokens=1,* delims=\" %%A in ("%~f0") do (
        if "%%~A"=="]" (
            goto :extract_all_deps_end
        )

        echo // Extracting archive "%%~B" to "%deps_dir%\%%~A"
        if not exist "%third_party_common_dir%\%%~A\%%~B" (
            call :err_msg "File not found"
            exit /b 1
        )
        for /f "usebackq tokens=* delims=" %%Z in ('"%%~nB"') do (
            if /i "%%~xZ%%~xB"==".tar.gz" (
                "%extractor%" x "%third_party_common_dir%\%%~A\%%~B" -so | "%extractor%" x -si -ttar -y -aoa -o"%deps_dir%\%%~A" || (
                    call :err_msg "Extraction failed"
                    exit /b 1
                )
            ) else (
                "%extractor%" x "%third_party_common_dir%\%%~A\%%~B" -y -aoa -o"%deps_dir%\%%~A" || (
                    call :err_msg "Extraction failed"
                    exit /b 1
                )
            )
        )

        for /f "tokens=* delims=" %%C in ('dir /b /a:d "%deps_dir%\%%~A\" 2^>nul') do (
            echo // Flattening dir "%deps_dir%\%%~A\%%~C" by moving everything inside it to "%deps_dir%\%%~A"
            robocopy /E /MOVE /MT4 /NS /NC /NFL /NDL /NP /NJH /NJS "%deps_dir%\%%~A\%%~C" "%deps_dir%\%%~A"
            if ERRORLEVEL 8 (
                call :err_msg "Failed to flatten dir of dep"
                exit /b 1
            )
            
            if exist "%deps_dir%\%%~A\%%~C" (
                echo // Removing nested dir "%deps_dir%\%%~A\%%~C"
                rmdir /s /q "%deps_dir%\%%~A\%%~C"
            )
        )
        
    )
:extract_all_deps_end
exit /b 0

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


deps_to_extract=[
libssq\libssq.tar.gz
zlib\zlib.tar.gz
curl\curl.tar.gz
protobuf\protobuf.tar.gz
mbedtls\mbedtls.tar.gz
]

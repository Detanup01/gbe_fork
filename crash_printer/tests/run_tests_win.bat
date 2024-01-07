@echo off

pushd "%~dp0"

call :cleanup

setlocal
call ..\..\build_win_set_env.bat 64
cl.exe /DEBUG:FULL /Z7 /Od /std:c++17 /DYNAMICBASE /errorReport:none /nologo /utf-8 /EHsc /GF /GL- /GS /MT /I../ ../win.cpp ../common.cpp test_win.cpp kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib Iphlpapi.lib Wldap32.lib Winmm.lib Bcrypt.lib Dbghelp.lib /link /DYNAMICBASE /ERRORREPORT:NONE /NOLOGO /OUT:test_win.exe && (
    call test_win.exe
    
    setlocal enableDelayedExpansion
    echo exit code = !errorlevel!
    endlocal

    call :cleanup
)
endlocal

setlocal
call ..\..\build_win_set_env.bat 32
cl.exe /DEBUG:FULL /Z7 /Od /std:c++17 /DYNAMICBASE /errorReport:none /nologo /utf-8 /EHsc /GF /GL- /GS /MT /I../ ../win.cpp ../common.cpp test_win.cpp kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib Iphlpapi.lib Wldap32.lib Winmm.lib Bcrypt.lib Dbghelp.lib /link /DYNAMICBASE /ERRORREPORT:NONE /NOLOGO /OUT:test_win.exe && (
    call test_win.exe
    
    setlocal enableDelayedExpansion
    echo exit code = !errorlevel!
    endlocal

    call :cleanup
)
endlocal

rmdir /s /q crash_test

popd

exit /b 0


:cleanup
    del /f /q test_win.exe >nul 2>&1
    del /f /q test_win.ilk >nul 2>&1
    del /f /q test_win.obj >nul 2>&1
    del /f /q test_win.pdb >nul 2>&1
    del /f /q win.obj >nul 2>&1
    del /f /q common.obj >nul 2>&1
exit /b

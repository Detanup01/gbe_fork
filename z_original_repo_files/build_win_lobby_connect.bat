@echo off
cd /d "%~dp0"
mkdir release\lobby_connect
del /Q release\lobby_connect\*
call build_set_protobuf_directories.bat
"%PROTOC_X86_EXE%" -I.\dll\ --cpp_out=.\dll\ .\dll\net.proto
call build_env_x86.bat
cl dll/rtlgenrandom.c dll/rtlgenrandom.def
cl /DNO_DISK_WRITES /DLOBBY_CONNECT /DEMU_RELEASE_BUILD /DNDEBUG /I%PROTOBUF_X86_DIRECTORY%\include\ lobby_connect.cpp dll/*.cpp dll/*.cc "%PROTOBUF_X86_LIBRARY%" Iphlpapi.lib Ws2_32.lib rtlgenrandom.lib Shell32.lib Comdlg32.lib /EHsc /MP12 /Ox /link /debug:none /OUT:release\lobby_connect\lobby_connect.exe
del /Q /S release\lobby_connect\*.lib
del /Q /S release\lobby_connect\*.exp
copy Readme_lobby_connect.txt release\lobby_connect\Readme.txt

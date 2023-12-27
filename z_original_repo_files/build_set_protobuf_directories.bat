@echo off
cd /d "%~dp0"
SET PROTOBUF_X86_DIRECTORY=..\vcpkg\installed\x86-windows-static
SET PROTOBUF_X64_DIRECTORY=..\vcpkg\installed\x64-windows-static

rem location of protoc in protobuf directories:
SET PROTOC_X86_EXE=%PROTOBUF_X86_DIRECTORY%\tools\protobuf\protoc.exe
SET PROTOC_X64_EXE=%PROTOBUF_X64_DIRECTORY%\tools\protobuf\protoc.exe

if exist "%PROTOBUF_X86_DIRECTORY%\lib\libprotobuf-lite.lib" (
SET PROTOBUF_X86_LIBRARY=%PROTOBUF_X86_DIRECTORY%\lib\libprotobuf-lite.lib
) else (
SET PROTOBUF_X86_LIBRARY=%PROTOBUF_X86_DIRECTORY%\lib\libprotobuf.lib
)

if exist "%PROTOBUF_X64_DIRECTORY%\lib\libprotobuf-lite.lib" (
SET PROTOBUF_X64_LIBRARY=%PROTOBUF_X64_DIRECTORY%\lib\libprotobuf-lite.lib
) else (
SET PROTOBUF_X64_LIBRARY=%PROTOBUF_X64_DIRECTORY%\lib\libprotobuf.lib
)

import os

def files_from_dir(dir, extension, filter=[]):
    out = []
    for f in os.listdir(dir):
        if f.endswith(extension) and f not in filter:
            out.append(os.path.join(dir, f))
    return out

def convert_to_obj(files, obj_dir):
    out = []
    for f in files:
        out.append(os.path.join(obj_dir, os.path.splitext(os.path.basename(f))[0] + ".obj"))
    return out

def cl_line_obj(arguments, out_dir):
    return "rmdir /S /Q {0}\nmkdir {0}\ncl /Fo:{0}/ /c {1}\n".format(out_dir, ' '.join(arguments))

def cl_line_link(arguments, linker_arguments):
    return "cl /LD {} /link {}\n".format(' '.join(arguments), ' '.join(linker_arguments))

def cl_line_exe(arguments, linker_arguments):
    return "cl {} /link {}\n".format(' '.join(arguments), ' '.join(linker_arguments))

jobs = 4
normal_build_args = ["/EHsc", "/Ox", "/MP{}".format(jobs)]

includes = ["ImGui", "overlay_experimental"]
includes_32 = list(map(lambda a: '/I{}'.format(a), ["%PROTOBUF_X86_DIRECTORY%\\include\\"] + includes))
includes_64 = list(map(lambda a: '/I{}'.format(a), ["%PROTOBUF_X64_DIRECTORY%\\include\\"] + includes))

debug_build_args = []
release_build_args = ["/DEMU_RELEASE_BUILD", "/DNDEBUG"]
steamclient_build_args = ["/DSTEAMCLIENT_DLL"]

experimental_build_args = ["/DEMU_EXPERIMENTAL_BUILD", "/DCONTROLLER_SUPPORT", "/DEMU_OVERLAY"]
steamclient_experimental_build_args = experimental_build_args + steamclient_build_args

normal_linker_libs = ["Iphlpapi.lib", "Ws2_32.lib", "rtlgenrandom.lib", "Shell32.lib"]
experimental_linker_libs = ["opengl32.lib", "Winmm.lib"] + normal_linker_libs
linker_32 = ['"%PROTOBUF_X86_LIBRARY%"']
linker_64 = ['"%PROTOBUF_X64_LIBRARY%"']

controller_deps = ["controller/gamepad.c"]
imgui_deps =  files_from_dir("ImGui", ".cpp") + ["ImGui/backends/imgui_impl_dx9.cpp", "ImGui/backends/imgui_impl_dx10.cpp", "ImGui/backends/imgui_impl_dx11.cpp", "ImGui/backends/imgui_impl_dx12.cpp", "ImGui/backends/imgui_impl_win32.cpp", "ImGui/backends/imgui_impl_vulkan.cpp", "ImGui/backends/imgui_impl_opengl3.cpp", "ImGui/backends/imgui_win_shader_blobs.cpp"]
proto_deps = list(map(lambda a: a.replace(".proto", ".pb.cc"), files_from_dir("dll", ".proto")))
all_deps = proto_deps + files_from_dir("detours", ".cpp") + controller_deps + imgui_deps + files_from_dir("overlay_experimental/System", ".cpp")

sc_different_deps = ["flat.cpp", "dll.cpp"]
steam_deps = files_from_dir("dll", ".cpp", sc_different_deps)
overlay_deps = files_from_dir("overlay_experimental", ".cpp") + files_from_dir("overlay_experimental/windows", ".cpp")
experimental_steam_deps = steam_deps + overlay_deps
sc_different_deps = list(map(lambda a: "dll/" + a, sc_different_deps))

regular_files = []

head = """@echo off
cd /d "%~dp0"
rmdir /S /Q release
mkdir release
mkdir release\experimental
mkdir release\experimental_steamclient
mkdir release\debug_experimental
mkdir release\debug_experimental_steamclient
call build_set_protobuf_directories.bat
"""

head_32bit = """"%PROTOC_X86_EXE%" -I.\dll\ --cpp_out=.\dll\ .\dll\\net.proto
call build_env_x86.bat
cl dll/rtlgenrandom.c dll/rtlgenrandom.def
"""

head_64bit = """"%PROTOC_X64_EXE%" -I.\dll\ --cpp_out=.\dll\ .\dll\\net.proto
call build_env_x64.bat
cl dll/rtlgenrandom.c dll/rtlgenrandom.def
"""

footer = """
copy Readme_release.txt release\Readme.txt
xcopy /s files_example\* release\\
copy Readme_experimental.txt release\experimental\Readme.txt
copy Readme_debug.txt release\debug_experimental\Readme.txt
copy steamclient_loader\ColdClientLoader.ini release\experimental_steamclient\\
call build_win_lobby_connect.bat
call build_win_find_interfaces.bat
"""

out = head
out += head_32bit

deps_folder = "deps"
sc_deps_folder = "deps_sc"

def generate_common(include_arch, linker_arch, steam_api_name, steamclient_name):
    out = ""
    out += cl_line_obj(normal_build_args + release_build_args + include_arch + all_deps, deps_folder)
    out += cl_line_link(normal_build_args + release_build_args + include_arch + steam_deps + sc_different_deps + ["deps/net.pb.obj"] + linker_arch + normal_linker_libs, ["/debug:none", "/OUT:release\\{}".format(steam_api_name)])

    debug_full_args = normal_build_args + debug_build_args + experimental_build_args + include_arch
    out += cl_line_obj(debug_full_args + experimental_steam_deps, sc_deps_folder)

    debug_full_dll_args = debug_full_args + sc_different_deps + convert_to_obj(all_deps, deps_folder) + convert_to_obj(experimental_steam_deps, sc_deps_folder) + linker_arch + experimental_linker_libs
    out += cl_line_link(debug_full_dll_args, ["/OUT:release\debug_experimental\\{}".format(steam_api_name)])
    out += cl_line_link(steamclient_build_args + debug_full_dll_args, ["/OUT:release\debug_experimental_steamclient\\{}".format(steamclient_name)])

    release_full_args = normal_build_args + release_build_args + experimental_build_args + include_arch
    out += cl_line_obj(release_full_args + experimental_steam_deps, sc_deps_folder)

    release_full_dll_args = release_full_args + sc_different_deps + convert_to_obj(all_deps, deps_folder) + convert_to_obj(experimental_steam_deps, sc_deps_folder) + linker_arch + experimental_linker_libs
    out += cl_line_link(release_full_dll_args, ["/debug:none", "/OUT:release\experimental\\{}".format(steam_api_name)])
    out += cl_line_link(steamclient_build_args + release_full_dll_args, ["/debug:none", "/OUT:release\experimental_steamclient\\{}".format(steamclient_name)])
    out += cl_line_link(release_build_args + experimental_build_args + ["steamclient.cpp"] + normal_build_args, ["/debug:none", "/OUT:release\experimental\\{}".format(steamclient_name)])
    return out

out += generate_common(includes_32, linker_32, "steam_api.dll", "steamclient.dll")

out += cl_line_exe(files_from_dir("steamclient_loader", ".cpp") + ["advapi32.lib", "user32.lib"] + normal_build_args, ["/debug:none", "/OUT:release\experimental_steamclient\steamclient_loader.exe"])

out += head_64bit
out += generate_common(includes_64, linker_64, "steam_api64.dll", "steamclient64.dll")


out += footer


with open("build_win_release_test.bat", "w") as f:
    f.write(out)

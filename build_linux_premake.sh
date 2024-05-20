#!/usr/bin/env bash

chmod +777 third-party/common/win/premake/premake5 
./third-party/common/win/premake/premake5 --os=linux genproto

# target other than clang?
./third-party/common/win/premake/premake5 --os=linux --cc=clang gmake2

# going into build dir
cd build/project/gmake2/linux
# you can select individual or all

# make config=debug_x32
make config=debug_x32
make config=debug_x64
make config=release_x32 
make config=release_x64 

cd ..
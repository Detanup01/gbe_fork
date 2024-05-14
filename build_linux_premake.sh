#!/usr/bin/env bash

# doto make a check here

curl -L  "https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-linux.tar.gz" --ssl-no-revoke --output premake.tar.gz
tar -xf premake.tar.gz

chmod +777 premake5 # do we really need this?
./premake5 --os=linux generateproto

# target other than clang?
./premake5 --os=linux --cc=clang gmake 

# going into build dir
cd GBE_Build
# you can select individual or all

# individual
#make config=debug_x32
# all
make

cd ..
protoc -I./dll/ --cpp_out=./dll/ ./dll/*.proto
clang++ -shared -fPIC -o libsteam_api.so dll/*.cpp dll/*.cc -g3 -Wno-return-type -fsanitize=address -lasan -lprotobuf-lite -std=c++14 && echo built 

.DEFAULT_GOAL := all

CXX=clang++
CXX_FLAGS += -fPIC -std=c++14
LD_FLAGS += -shared -lprotobuf-lite -ldl -Wl,--no-undefined
LIBRARY_NAME=libsteam_api.so
RM = rm -f

PROTOS      := $(wildcard dll/*.proto)
PROTO_CC    := $(PROTOS:.proto=.pb.cc)
PROTO_OBJ   := $(PROTOS:.proto=.pb.o)
PROTO_HEADERS := $(PROTOS:.proto=.pb.h)
HEADERS     := $(wildcard dll/*.h) $(PROTO_HEADERS)
SRC_NOPROTO := $(wildcard dll/*.cpp)
SRC         := $(SRC_NOPROTO) $(PROTO_CC)
OBJ_NOPROTO := $(SRC_NOPROTO:.cpp=.o)
OBJ         := $(OBJ_NOPROTO) $(PROTO_OBJ)

$(PROTO_CC) : $(PROTOS)
$(PROTO_HEADERS) : $(PROTO_CC)
$(OBJ_NOPROTO) : $(PROTO_CC) $(PROTO_HEADERS)
$(OBJ) : $(HEADERS)

release: CXX_FLAGS += -DNDEBUG -DEMU_RELEASE_BUILD -Ofast
release: LD_FLAGS += -lpthread
release32: CXX_FLAGS += -m32
release32: LD_FLAGS += -m32
debug: CXX_FLAGS += -g3 -fsanitize=address
debug: LD_FLAGS += -lasan
release: library
release32: release
debug: library
all: release

library: $(LIBRARY_NAME)

$(PROTO_CC) :
	protoc -I./dll/ --cpp_out=./dll/ ./dll/*.proto

$(PROTO_OBJ) : %.o : %.cc
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_NOPROTO) : %.o : %.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(LIBRARY_NAME): $(OBJ)
	$(CXX) $(CXX_FLAGS) -o $@ $^ $(LD_FLAGS)

clean:
	$(RM) $(OBJ) $(LIBRARY_NAME) $(PROTO_CC) $(PROTOS:.proto=.pb.h)

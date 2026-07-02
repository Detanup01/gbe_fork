set(CMAKE_C_COMPILER clang CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER clang++ CACHE INTERNAL "")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden" CACHE STRING "" FORCE)

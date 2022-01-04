cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(proto)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes -Werror -fPIC")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("CMAKE_BUILD_TYPE: Debug")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og")
else()
    message("CMAKE_BUILD_TYPE: Release")
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
endif()

set(EXTRALIBS glog gflags protobuf boost_system boost_filesystem)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

file(GLOB_RECURSE CXX_SRCS "${CMAKE_SOURCE_DIR}/*.cc")
add_library(proto STATIC ${CXX_SRCS})
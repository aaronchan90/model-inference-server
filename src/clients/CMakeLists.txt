cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(model_inferece_server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes -Werror -Wno-deprecated")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("CMAKE_BUILD_TYPE: Debug")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og")
else()
    message("CMAKE_BUILD_TYPE: Release")
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
endif()

find_package(Threads REQUIRED)
find_package(Boost)

set(EXTRALIBS proto)
set(EXTRALIBS ${EXTRALIBS} glog gflags protobuf grpc++ grpc grpc++_reflection yaml-cpp boost_system boost_filesystem crc32c rt ${CMAKE_THREAD_LIBS_INIT})

include_directories(${CMAKE_SOURCE_DIR})
link_directories(/usr/local/lib)
link_directories(${CMAKE_SOURCE_DIR}/../proto/lib)

#file(GLOB CXX_SRCS "${CMAKE_SOURCE_DIR}/example_client.cc")

#add_library(client ${CXX_SRCS})
add_executable(example_client ${CMAKE_SOURCE_DIR}/example_client.cc)
target_link_libraries(example_client ${EXTRALIBS})

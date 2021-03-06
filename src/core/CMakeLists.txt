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

set(EXTRALIBS proto utils)
set(EXTRALIBS ${EXTRALIBS} glog gflags protobuf grpc++ grpc grpc++_reflection yaml-cpp boost_system boost_filesystem crc32c rt ${CMAKE_THREAD_LIBS_INIT})

include_directories(${CMAKE_SOURCE_DIR})

link_directories(/usr/local/lib)
link_directories(${CMAKE_SOURCE_DIR}/../utils/lib)
link_directories(${CMAKE_SOURCE_DIR}/../proto/lib)

file(GLOB CXX_SRCS "${CMAKE_SOURCE_DIR}/*.cc")
file(GLOB model_manage "${CMAKE_SOURCE_DIR}/model_manager/*.cc")
file(GLOB scheduler "${CMAKE_SOURCE_DIR}/scheduler/*.cc")
file(GLOB backend "${CMAKE_SOURCE_DIR}/backend/*.cc")
file(GLOB demo_backend "${CMAKE_SOURCE_DIR}/backend/demo/*.cc")
file(GLOB server "${CMAKE_SOURCE_DIR}/server/*.cc")
list(APPEND CXX_SRCS ${model_manage} ${server} ${scheduler} ${backend} ${demo_backend})

list(REMOVE_ITEM CXX_SRCS ${CMAKE_SOURCE_DIR}/main.cc)

add_library(core ${CXX_SRCS})

add_executable(model_inference_server ${CMAKE_SOURCE_DIR}/main.cc)
target_link_libraries(model_inference_server core ${EXTRALIBS})

add_executable(model_manager_test ${CMAKE_SOURCE_DIR}/tests/model_manager_test.cc)
target_link_libraries(model_manager_test core ${EXTRALIBS})

add_executable(demo_backend_test ${CMAKE_SOURCE_DIR}/tests/demo_backend_test.cc)
target_link_libraries(demo_backend_test core ${EXTRALIBS})
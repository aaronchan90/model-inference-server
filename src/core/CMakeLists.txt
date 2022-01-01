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

include_directories(${CMAKE_SOURCE_DIR})

file(GLOB CXX_SRCS "${CMAKE_SOURCE_DIR}/*.cc")

list(REMOVE_ITEM CXX_SRCS ${CMAKE_SOURCE_DIR}/main.cc)

add_library(core ${CXX_SRCS})

add_executable(model_inferece_server main.cc)
target_link_libraries(inference_engine core)

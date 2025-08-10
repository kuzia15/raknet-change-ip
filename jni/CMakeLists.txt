cmake_minimum_required(VERSION 3.12)
project(wnrpc)

set(CMAKE_CXX_STANDARD 20)

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    add_definitions(-DVER_x32=true)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    add_definitions(-DVER_x32=false)
endif()

message(WARNING "-------------------------------------------------")
message(WARNING "")
message(WARNING "Current: libwnrpc.so [${ANDROID_ABI}]")
message(WARNING "")
message(WARNING "-------------------------------------------------")

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/arm/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vendor/RakNet/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vendor/RakNet/SAMP/*.cpp
)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -s -fvisibility=hidden -pthread -Wall -fpack-struct=1 -O2 -fexceptions")

add_library(wnrpc SHARED ${SOURCES})

target_link_libraries(wnrpc
    log
    android
    EGL
    GLESv2
)
